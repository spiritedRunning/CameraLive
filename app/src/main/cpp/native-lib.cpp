#include <jni.h>
#include <string>
#include <pthread.h>
#include "librtmp/rtmp.h"
#include "JavaCallHelper.h"


JavaVM *javaVM = 0;
pthread_t pid;
JavaCallHelper *helper = 0;
RTMP *rtmp = 0;
uint64_t startTime;

char *path = 0;

// init
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_4;
}

void *connect(void *args) {
    int ret;
    rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);

    do {
        // 解析url地址，可能失败（地址不合法）
        ret = RTMP_SetupURL(rtmp, path);
        if (!ret) {
            break;
        }

        RTMP_EnableWrite(rtmp);
        if (!(ret = RTMP_Connect(rtmp, nullptr))) break;
        if (!(ret = RTMP_ConnectStream(rtmp, 0))) break;
    } while (false);

    if (!ret) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = 0;
    }

    delete(path);
    path = 0;

    // 通知Java开始推流
    helper->onPrepare(ret);
    startTime = RTMP_GetTime();
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_connect(JNIEnv *env, jobject thiz, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    path = new char[strlen(url) + 1];
    strcpy(path, url);

    pthread_create(&pid, 0, connect, 0);
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_disConnect(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeInit(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeDeInit(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_initVideoEnv(JNIEnv *env, jobject thiz, jint width,
                                                    jint height, jint fps, jint bit_rate) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_releaseVideoEnv(JNIEnv *env, jobject thiz) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeSendVideo(JNIEnv *env, jobject thiz,
                                                       jbyteArray buffer) {

}