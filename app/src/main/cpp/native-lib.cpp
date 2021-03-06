#include <jni.h>
#include <string>
#include <pthread.h>
#include "librtmp/rtmp.h"
#include "JavaCallHelper.h"
#include "VideoChannel.h"
#include "log.h"
#include "AudioChannel.h"


JavaVM *javaVM = 0;
pthread_t pid;
JavaCallHelper *helper = 0;
RTMP *rtmp = 0;
uint64_t startTime;

VideoChannel *videoChannel = 0;
AudioChannel *audioChannel = 0;

char *path = 0;

pthread_mutex_t mutex;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_4;
}


void callback(RTMPPacket *packet) {
    if (rtmp) {
        packet->m_nInfoField2 = rtmp->m_stream_id;
        packet->m_nTimeStamp = RTMP_GetTime() - startTime;
        RTMP_SendPacket(rtmp, packet, 1);
    }

    RTMPPacket_Free(packet);
    delete packet;
}

void *connect(void *args) {
    int ret;
    rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);

    LOGE("CONNECT path: %s", path);
    do {
        // 解析url地址，可能失败（地址不合法）
        ret = RTMP_SetupURL(rtmp, path);
        if (!ret) {
            LOGE("setup url failed");
            break;
        }

        RTMP_EnableWrite(rtmp);
        LOGE("rtmp connect");
        if (!(ret = RTMP_Connect(rtmp, 0))) break;
        LOGE("rtmp connect stream");
        if (!(ret = RTMP_ConnectStream(rtmp, 0))) break;
        LOGE("rtmp connect succ");

        // 发送audio specific config
        RTMPPacket *packet = audioChannel->getAudioConfig();
        callback(packet);
    } while (false);

    if (!ret) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = 0;
    }

    delete(path);
    path = 0;

    LOGE("rtmp connect state: %d", ret);
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
    pthread_mutex_lock(&mutex);
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = 0;
    }

    if (videoChannel) {
        videoChannel->resetPts();
    }

    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeInit(JNIEnv *env, jobject thiz) {
    helper = new JavaCallHelper(javaVM, env, thiz);
    pthread_mutex_init(&mutex, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeDeInit(JNIEnv *env, jobject thiz) {
    if (helper) {
        delete(helper);
        helper = 0;
    }
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_initVideoEnv(JNIEnv *env, jobject thiz, jint width,
                                                    jint height, jint fps, jint bit_rate) {
    videoChannel = new VideoChannel;
    videoChannel->openCodec(width, height, fps, bit_rate);
    videoChannel->setCallback(callback);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_releaseVideoEnc(JNIEnv *env, jobject thiz) {
    if (videoChannel) {
        delete videoChannel;
        videoChannel = 0;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeSendVideo(JNIEnv *env, jobject thiz,
                                                       jbyteArray buffer) {
    LOGE("native send video start");
    jbyte  *data = env->GetByteArrayElements(buffer, 0);

    pthread_mutex_lock(&mutex);
    videoChannel->encode(reinterpret_cast<uint8_t *>(data));
    pthread_mutex_unlock(&mutex);

    env->ReleaseByteArrayElements(buffer, data, 0);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_example_cameralive_RtmpClient_initAudioEnc(JNIEnv *env, jobject thiz, jint sample_rate,
                                                    jint channels) {
    audioChannel = new AudioChannel();
    audioChannel->setCallback(callback);
    audioChannel->openCodec(sample_rate, channels);
    return audioChannel->getInputByteNum();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_nativeSendAudio(JNIEnv *env, jobject thiz, jbyteArray buffer,
                                                       jint len) {
    jbyte *data = env->GetByteArrayElements(buffer, 0);
    pthread_mutex_lock(&mutex);
    audioChannel->encode(reinterpret_cast<int32_t *>(data), len);
    pthread_mutex_unlock(&mutex);

    env->ReleaseByteArrayElements(buffer, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameralive_RtmpClient_releaseAudioEnc(JNIEnv *env, jobject thiz) {
   if (audioChannel) {
       delete audioChannel;
       audioChannel = 0;
   }
}

