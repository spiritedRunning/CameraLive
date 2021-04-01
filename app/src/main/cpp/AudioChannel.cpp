//
// Created by Zach on 2021/4/1.
//

#include <malloc.h>
#include <cstring>
#include "AudioChannel.h"

AudioChannel::AudioChannel() {

}

AudioChannel::~AudioChannel() {
    closeCodec();
    if (outputBuffer) {
        free(outputBuffer);
        outputBuffer = 0;
    }
}

void AudioChannel::openCodec(int sampleRate, int channels) {
    unsigned long inputSamples;  // 输入样本，要送给编码器编码的样本数
    codec = faacEncOpen(sampleRate, channels, &inputSamples, &maxOutputBytes);
    inputByteNum = inputSamples * 2; // 样本是16位，一个样本2字节
    outputBuffer = static_cast<unsigned char *>(malloc(maxOutputBytes));

    faacEncConfigurationPtr configurationPtr = faacEncGetCurrentConfiguration(codec);
    configurationPtr->mpegVersion = MPEG4;
    configurationPtr->aacObjectType = LOW;
    // 0: 编码出aac裸数据
    // 1: 每一帧音频编码的结果数据 都会携带ADTS（包含了采样、声道等信息的一个数据头）
    configurationPtr->outputFormat = 0;
    configurationPtr->inputFormat = FAAC_INPUT_16BIT;

    faacEncSetConfiguration(codec, configurationPtr);
}


void AudioChannel::encode(int32_t *data, int len) {
    int bytelen = faacEncEncode(codec, data, len, outputBuffer, maxOutputBytes);
    if (bytelen > 0) {
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, bytelen + 2);
        packet->m_body[0] = 0xAF;
        packet->m_body[1] = 0x01;

        memcpy(&packet->m_body[2], outputBuffer, bytelen);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bytelen + 2;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;  // 分配一个和视频通道不同的
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        callback(packet);
    }
}


/**
 * 拿到audio specific config. 音频只需要在发送声音数据包之前发送一个就可以
 */
RTMPPacket *AudioChannel::getAudioConfig() {
    u_char *buf;
    u_long len;
    faacEncGetDecoderSpecificInfo(codec, &buf, &len);
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, len + 2);
    packet->m_body[0] = 0xAF;
    packet->m_body[1] = 0x00;  // audio specific config固定配置

    memcpy(&packet->m_body[2], buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = len + 2;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;  // 分配一个和视频通道不同的
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    return packet;
}

void AudioChannel::closeCodec() {
    if (codec) {
        faacEncClose(codec);
        codec = 0;
    }

}
