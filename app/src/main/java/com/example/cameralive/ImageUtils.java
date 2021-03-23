package com.example.cameralive;

import android.graphics.ImageFormat;
import android.util.Log;

import androidx.camera.core.ImageProxy;

import java.nio.ByteBuffer;

public class ImageUtils {
    private static final String TAG = "ImageUtils";

    static {
        System.loadLibrary("ImageUtils");
    }

    private static ByteBuffer planeBuffer;
    private static byte[] result;

    public static byte[] getBytes(ImageProxy image, int rotationDegrees, int width, int height) {
        int format = image.getFormat();
        if (format != ImageFormat.YUV_420_888) {
            // https://developer.android.google.cn/training/camerax/analyze
            throw new IllegalStateException("not support image format!");
        }

        ByteBuffer yuv420 = ByteBuffer.allocate(image.getWidth() * image.getHeight() * 3 / 2);
        ImageProxy.PlaneProxy[] planes = image.getPlanes();


        /**
         * Y 数据
         */
        int pixelStride = planes[0].getPixelStride();  // Y 只会是1
        ByteBuffer yBuffer = planes[0].getBuffer();
        int rowStride = planes[0].getRowStride();

        // 1. rowStride == width,  则是一个空数组
        // 2. rowStride > width, 是每行为了字节对齐，多出来的空字节
        byte[] skipRow = new byte[rowStride - image.getWidth()];

        byte[] row = new byte[image.getWidth()];
        for (int i = 0; i < image.getHeight(); i++) {
            yBuffer.get(row);
            yuv420.put(row);

            // 最后一行因为后面是 U数据，后面没有无效数据，无需跳过
            if (i < image.getHeight() - 1) {
                yBuffer.get(skipRow);
            }
        }


        try {

            /**
             *  U, V数据
             */
            for (int i = 1; i < 3; i++) {
                ImageProxy.PlaneProxy plane = planes[i];
                pixelStride = plane.getPixelStride();
                rowStride = plane.getRowStride();
                planeBuffer = plane.getBuffer();

//                Log.e(TAG, "planeBuffer size: " + planeBuffer.array().length);

                int uvWidth = image.getWidth() / 2;
                int uvHeight = image.getHeight() / 2;

                Log.e(TAG, "i = " + i + " uvHeight:" + uvHeight + ", rowStride: " + rowStride);

                for (int j = 0; j < uvHeight; j++) {
                    for (int k = 0; k < rowStride; k++) {
                        if (j == uvHeight - 1) {   // 单独处理最后一行
                            if (pixelStride == 1) { // uv没混合
                                if (k >= uvWidth) break;
                            } else if (pixelStride == 2) { // uv混合
                                if (k >= image.getWidth() ) break;
                            }
                        }
                        if (planeBuffer.remaining() == 0) {
                            // 在 pos: 153599, j = 239, k = 639 时，get会越界。 此处防止越界
                            break;
                        }

                        byte b = planeBuffer.get();
                        if (pixelStride == 1) {
                            if (k < uvWidth) {
                                yuv420.put(b);
                            }
                        } else if (pixelStride == 2) {
                            // 1. 偶数位下标是我们要的U/V数据
                            // 2. 丢弃无效占位数据
                            if (k < image.getWidth() && k % 2 == 0) {
                                yuv420.put(b);
                            }
                        }
                    }
                }
            }


            result = yuv420.array();
            if (rotationDegrees == 90 || rotationDegrees == 270) {
                result = rotation(result, image.getWidth(), image.getHeight(), rotationDegrees);
            }
        } catch (Exception e) {
            Log.e(TAG, "BufferUnderflowException pos: " + planeBuffer.position());
        }

        return result;
    }


    private static native byte[] rotation(byte[] data, int width, int height, int degrees);
}
