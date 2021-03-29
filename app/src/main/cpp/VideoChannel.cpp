//
// Created by Zach on 2021/3/25.
//

#include "VideoChannel.h"
#include <rtmp.h>
#include <android/log.h>
#include <x264.h>
#include <alloca.h>
#include <cstring>


VideoChannel::VideoChannel() {

}

VideoChannel::~VideoChannel() {
    if (codec) {
        x264_encoder_close(codec);
        codec = 0;
    }
}

void x264_log_default(void *, int i_level, const char *psz, va_list list) {
    __android_log_vprint(ANDROID_LOG_ERROR, "x264", psz, list);
}


void VideoChannel::openCodec(int width, int height, int fps, int bitrate) {
    x264_param_t param;

    // ultrafast: 编码速度与质量控制，使用最快的模式编码
    // zerolatency: 无延迟编码，实时通信方面
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    // base_line 3.2 编码规格 无B帧(数据量最小，但解码速度最慢)
    param.i_level_idc = 32;
    // 输入数据格式
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    // 无b帧
    param.i_bframe = 0;
    // 码率控制，CQP(恒定质量) CRF(恒定码率) ABR(平均码率)
    param.rc.i_rc_method = X264_RC_ABR;
    // 码率（比特率 kbps）
    param.rc.i_bitrate = bitrate / 1000;
    // 瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    // 帧率
    param.i_fps_num = fps;
    param.i_fps_den = 1;

    param.pf_log = x264_log_default;      // 输出编码日志
    // 帧距离（关键帧） 2s 1个关键帧
    param.i_keyint_max = fps * 2;
    // 是否复制sps和pps放在每个关键帧前面 该参数设置让每个关键帧（I帧）都附带sps/pps
    param.b_repeat_headers = 1;
    // 不使用并行编码  zerolatency场景下设置 Param.rc.i_lookahead = 0.  编码器来一帧编码一帧，无并行、无延迟
    param.i_threads = 1;
    param.rc.i_lookahead = 0;
    x264_param_apply_profile(&param, "baseline");

    codec = x264_encoder_open(&param);
    ySize = width * height;
    uSize = (width >> 1) * (height >> 1);
    this->width = width;
    this->height = height;


}

void VideoChannel::encode(uint8_t *data) {
    x264_picture_t pic_in;   // 待编码数据
    x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);

    pic_in.img.plane[0] = data;
    pic_in.img.plane[1] = data + ySize;
    pic_in.img.plane[2] = data + ySize + uSize;
    // 编码的i_pts, 每次需要增长
    pic_in.i_pts = i_pts++;

    x264_picture_t pic_out;
    x264_nal_t *pp_nal;
    int pi_nal;  // 输出了多少nal

    int error = x264_encoder_encode(codec, &pp_nal, &pi_nal, &pic_in, &pic_out);
    if (error <= 0) {
        return;
    }
    int spslen, ppslen;
    uint8_t *sps, *pps;
    for (int i = 0; i < pi_nal; i++) {
        int type = pp_nal[i].i_type;
        uint8_t *p_payload = pp_nal[i].p_payload;
        int i_payload = pp_nal[i].i_payload;
        if (type == NAL_SPS) { // sps 后肯定跟着pps
            spslen = i_payload - 4;  // 去掉间隔 00 00 00 01
            sps = (uint8_t *) alloca(spslen);  // 栈中申请内存
            memcpy(sps, p_payload + 4, spslen);
        } else if (type == NAL_PPS) {
            ppslen = i_payload - 4;  // 去掉间隔 00 00 00 01
            pps = (uint8_t *) alloca(ppslen);
            memcpy(pps, p_payload + 4, ppslen);

            // pps后肯定是I帧，发送I帧之前要发一个sps与pps
            sendVideoConfig(sps, pps, spslen, ppslen);
        } else {
            sendFrame(type, p_payload, i_payload);
        }
    }
}


void VideoChannel::sendVideoConfig(uint8_t *sps, uint8_t *pps, int spslen, int ppslen) {
    int bodySize = 13 + spslen + 3 + ppslen;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);

    int i = 0;
    packet->m_body[i++] = 0x17;  // 固定头
    packet->m_body[i++] = 0x00;  // 类型：AVC序列头，指导播放器如何解码
    // 合成时间： composition time 0x000000
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    packet->m_body[i++] = 0x01;  // 版本
    // 编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];

    packet->m_body[i++] = 0xFF; // 几个字节表示NALU长度， 包长为 (0xFF & 3) + 1=4, 也就是4个字节表示
    packet->m_body[i++] = 0xE1; // sps个数， 个数为0xE1 & 0x1F = 1
    // sps长度
    packet->m_body[i++] = (spslen >> 8) & 0xFF;
    packet->m_body[i++] = spslen & 0xFF;
    // sps内容
    memcpy(&packet->m_body[i], sps, spslen);
    i += spslen;

    // pps
    packet->m_body[i++] = 0x01;  // pps个数，固定1
    // pps长度
    packet->m_body[i++] = (ppslen >> 8) & 0xFF;
    packet->m_body[i++] = ppslen & 0xFF;
    // pps内容
    memcpy(&packet->m_body[i], pps, ppslen);

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySize;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nTimeStamp = 0;  // sps与pps（不是图像），没有时间戳
    packet->m_nChannel = 0x10; // 随便给一个通道，避免和rtmp.c中重复就可以

    callback(packet);
}

void VideoChannel::sendFrame(int type, uint8_t *p_payload, int i_payload) {
    // 去掉 00 00 00 01 / 00 00 01
    if (p_payload[2] == 0x00) {
        i_payload -= 4;
        p_payload += 4;
    } else if (p_payload[2] == 0x01) {
        i_payload -= 3;
        p_payload += 3;
    }

    RTMPPacket *packet = new RTMPPacket;
    int bodySize = 9 + i_payload;
    RTMPPacket_Alloc(packet, bodySize);
    RTMPPacket_Reset(packet);

    packet->m_body[0] = 0x27;  // 非关键帧
    if (type == NAL_SLICE_IDR) {
        packet->m_body[0] = 0x17;  // 关键帧
    }
    packet->m_body[1] = 0x01; // 类型
    // 合成时间
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;
    // 数据长度， 4个字节
    packet->m_body[5] = (i_payload >> 24) & 0xFF;
    packet->m_body[6] = (i_payload >> 16) & 0xFF;
    packet->m_body[7] = (i_payload >> 8) & 0xFF;
    packet->m_body[8] = i_payload & 0xFF;

    // 图片数据
    memcpy(&packet->m_body[9], p_payload, i_payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    callback(packet);
}


void VideoChannel::setCallback(Callback callback) {
    this->callback = callback;
}
