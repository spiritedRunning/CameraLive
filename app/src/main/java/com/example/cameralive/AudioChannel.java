package com.example.cameralive;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.HandlerThread;



/**
 * Created by Zach on 2021/4/1 13:59
 */
public class AudioChannel {

    private int sampleRate;
    private RtmpClient rtmpClient;

    private AudioRecord audioRecord;
    private int channelConfig;
    private byte[] buffer;
    private int minBufferSize;

    private Handler handler;
    private HandlerThread handlerThread;


    public AudioChannel(int sampleRate, int channels, RtmpClient rtmpClient) {
        this.rtmpClient = rtmpClient;
        this.sampleRate = sampleRate;

        channelConfig = channels == 2 ? AudioFormat.CHANNEL_IN_STEREO : AudioFormat.CHANNEL_IN_MONO;
        minBufferSize = AudioRecord.getMinBufferSize(sampleRate, channelConfig, AudioFormat.ENCODING_PCM_16BIT);

        handlerThread = new HandlerThread("AudioChannel-Thread");
        handlerThread.start();
        handler = new Handler(handlerThread.getLooper());
    }

    public void start() {
        handler.post(new Runnable() {
            @Override
            public void run() {
                audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate, channelConfig,
                        AudioFormat.ENCODING_PCM_16BIT, minBufferSize);
                audioRecord.startRecording();

                while (audioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
                    int len = audioRecord.read(buffer, 0, buffer.length);
                    if (len > 0) {
                        // 由于采样位为16位，样本数 = 字节数 / 2
                        rtmpClient.sendAudio(buffer, len >> 1);
                    }
                }
            }
        });
    }

    public void stop() {
        audioRecord.stop();
    }

    public void setInputByteNum(int inputByteNum) {
        buffer = new byte[inputByteNum];
        minBufferSize = Math.max(inputByteNum, minBufferSize);
    }

    public void release() {
        handlerThread.quitSafely();
        handler.removeCallbacksAndMessages(null);
        audioRecord.release();
    }
}
