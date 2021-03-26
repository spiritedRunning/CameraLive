//
// Created by Zach on 2021/3/25.
//

#include "VideoChannel.h"
#include <rtmp.h>
#include <android/log.h>
#include <x264.h>


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
    // 是否复制


}

void VideoChannel::encode(uint8_t *data) {

}


void VideoChannel::sendVideoConfig(uint8_t *sps, uint8_t *pps, int spslen, int ppslen) {

}

void VideoChannel::sendFrame(int type, uint8_t *payload, int iPayload) {

}


void VideoChannel::setCallback(Callback callback) {
    this->callback = callback;
}
