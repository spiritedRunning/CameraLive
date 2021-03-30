//
// Created by Administrator on 2021/3/29.
//

#ifndef CAMERALIVE_LOG_H
#define CAMERALIVE_LOG_H

#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "CAMERA_LIVE", __VA_ARGS__)


#endif //CAMERALIVE_LOG_H
