//
// Created by Zach on 2021/3/25.
//

#ifndef CAMERALIVE_VIDEOCHANNEL_H
#define CAMERALIVE_VIDEOCHANNEL_H

#include "x264.h"
#include "Callback.h"

/**
 * 封装x264编码操作
 */
class VideoChannel {
public:
    VideoChannel();
    ~VideoChannel();
    void encode(uint8_t *data);


public:
    void openCodec(int width, int height, int fps, int bitrate);
    void setCallback(Callback callback);
    void resetPts() {
        i_pts = 0;
    };

private:
    x264_t *codec = 0;
    int ySize;
    int uSize;
    int64_t i_pts = 0;
    Callback callback;

    int width;
    int height;

    void sendVideoConfig(uint8_t *sps, uint8_t *pps, int spslen, int ppslen);
    void sendFrame(int type, uint8_t *payload, int iPayload);

};


#endif //CAMERALIVE_VIDEOCHANNEL_H
