#include <jni.h>
#include <string>



extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_connect(JNIEnv *env, jobject thiz, jstring url) {
    // TODO: implement connect()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_disConnect(JNIEnv *env, jobject thiz) {
    // TODO: implement disConnect()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeInit(JNIEnv *env, jobject thiz) {
    // TODO: implement nativeInit()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeDeInit(JNIEnv *env, jobject thiz) {
    // TODO: implement nativeDeInit()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_initVideoEnv(JNIEnv *env, jobject thiz, jint width,
                                                    jint height, jint fps, jint bit_rate) {
    // TODO: implement initVideoEnv()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_releaseVideoEnv(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseVideoEnv()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeSendVideo(JNIEnv *env, jobject thiz,
                                                       jbyteArray buffer) {
    // TODO: implement nativeSendVideo()
}