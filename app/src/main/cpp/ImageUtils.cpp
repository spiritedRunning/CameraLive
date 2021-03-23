//
// Created by Zach on 2021/3/13.
//


#include <jni.h>
#include <libyuv.h>


extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_cameralive_ImageUtils_rotation(JNIEnv *env, jclass clazz, jbyteArray data_,
                                                jint width, jint height, jint degrees) {

    jbyte *data = env->GetByteArrayElements(data_, 0);
    uint8_t *src = reinterpret_cast<uint8_t *>(data);
    int ySize = width * height;
    int uSize = (width >> 1) * (height >> 1);
    int size = ySize * 3 >> 1;

    uint8_t dst[size];

    uint8_t *src_y = src;
    uint8_t *src_u = src + ySize;
    uint8_t *src_v = src + ySize + uSize;

    uint8_t *dst_y = dst;
    uint8_t *dst_u = dst + ySize;
    uint8_t *dst_v = dst + ySize + uSize;

    libyuv::I420Rotate(src_y, width, src_u, width >> 1, src_v, width >> 1,
                    dst_y, height, dst_u, height >> 1, dst_v, height >> 1,
                    width, height, static_cast<libyuv::RotationMode>(degrees));

    jbyteArray result = env->NewByteArray(size);
    env->SetByteArrayRegion(result, 0, size, reinterpret_cast<const jbyte *>(dst));

    env->ReleaseByteArrayElements(data_, data, 0);
    return result;

}
