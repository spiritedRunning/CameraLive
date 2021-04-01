package com.example.cameralive;

import android.util.Log;
import android.view.TextureView;

import androidx.lifecycle.LifecycleOwner;

public class RtmpClient {
    private static final String TAG = "RtmpClient";

    static {
        System.loadLibrary("native-lib");
    }

    private final LifecycleOwner lifecycleOwner;

    private int width;
    private int height;
    private boolean isConnected;
    private VideoChannel videoChannel;
    private AudioChannel audioChannel;

    public RtmpClient(LifecycleOwner lifecycleOwner) {
        this.lifecycleOwner = lifecycleOwner;
        nativeInit();
    }

    public void initVideo(TextureView displayer, int width, int height, int fps, int birRate) {
        this.width = width;
        this.height = height;
        videoChannel = new VideoChannel(this.lifecycleOwner, displayer, this);
        initVideoEnv(width, height, fps, birRate);
    }

    public void initAudio(int sampleRate, int channels) {
        audioChannel = new AudioChannel(sampleRate, channels, this);
        int inputByteNum = initAudioEnc(sampleRate, channels);
        audioChannel.setInputByteNum(inputByteNum);
    }

    public void toggleCamera() {
        videoChannel.toggleCamera();
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public boolean isConnected() {
        return isConnected;
    }

    public void startLive(String url) {
        connect(url);
    }

    public void stopLive() {
        isConnected = false;
        audioChannel.stop();
        disConnect();
        Log.e(TAG, "stop live...");
    }

    /**
     * JNICall
     */
    private void onPrepare(boolean isConnected) {
        this.isConnected = isConnected;

        audioChannel.start();
        Log.e(TAG, "can start live: " + isConnected);
    }

    public void sendVideo(byte[] buffer) {
        nativeSendVideo(buffer);
    }

    public void sendAudio(byte[] buffer, int sample_len) {
        nativeSendAudio(buffer, sample_len);
    }

    public void release() {
        videoChannel.release();
        audioChannel.release();
        releaseVideoEnc();
        releaseAudioEnc();
        nativeDeInit();
    }


    private native void connect(String url);

    private native void disConnect();

    private native void nativeInit();
    private native void nativeDeInit();

    private native void initVideoEnv(int width, int height, int fps, int bitRate);

    private native void releaseVideoEnc();

    private native void nativeSendVideo(byte[] buffer);


    private native int initAudioEnc(int sampleRate, int channels);

    private native void releaseAudioEnc();

    private native void nativeSendAudio(byte[] buffer, int len);
}
