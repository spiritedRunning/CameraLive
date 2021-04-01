//
// Created by Zach on 2021/4/1.
//

#ifndef CAMERALIVE_AUDIOCHANNEL_H
#define CAMERALIVE_AUDIOCHANNEL_H

#include <cstdint>
#include "Callback.h"
#include <faac.h>

class AudioChannel {
public:
    AudioChannel();
    ~AudioChannel();
    void encode(int32_t *data, int len);

public:
    void openCodec(int sampleRate, int channels);

    int getInputByteNum() {
        return inputByteNum;
    }

    void setCallback(Callback callback) {
        this->callback = callback;
    }

    RTMPPacket *getAudioConfig();

private:
    Callback callback;
    faacEncHandle codec = 0;
    unsigned long inputByteNum;
    unsigned char *outputBuffer = 0;
    unsigned long maxOutputBytes;

private:
    void closeCodec();
};


#endif //CAMERALIVE_AUDIOCHANNEL_H
