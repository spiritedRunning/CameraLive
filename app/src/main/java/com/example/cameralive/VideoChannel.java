package com.example.cameralive;

import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.TextureView;
import android.view.ViewGroup;

import androidx.camera.core.CameraX;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageAnalysisConfig;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.core.PreviewConfig;
import androidx.lifecycle.LifecycleOwner;

import java.io.FileOutputStream;


public class VideoChannel  implements Preview.OnPreviewOutputUpdateListener, ImageAnalysis.Analyzer {

    private CameraX.LensFacing currentFacing = CameraX.LensFacing.BACK;
    private LifecycleOwner lifeCycleOwner;
    private TextureView displayer;
    private RtmpClient rtmpClient;
    private HandlerThread handlerThread;

    public VideoChannel(LifecycleOwner lifeCycleOwner, TextureView displayer, RtmpClient rtmpClient) {
        this.lifeCycleOwner = lifeCycleOwner;
        this.rtmpClient = rtmpClient;
        this.displayer = displayer;

        handlerThread = new HandlerThread("Analysis-Thread");
        handlerThread.start();
        CameraX.bindToLifecycle(lifeCycleOwner, getPreview(), getAnalysis());

//        try {
//            fos = new FileOutputStream("/sdcard/a.yuv");
//        } catch (FileNotFoundException e) {
//            e.printStackTrace();
//        }
    }

    private Preview getPreview() {
        PreviewConfig previewConfig = new PreviewConfig.Builder().setTargetResolution(new Size(640, 480))
                .setLensFacing(currentFacing)
                .build();

        Preview preview = new Preview(previewConfig);
        preview.setOnPreviewOutputUpdateListener(this);
        return preview;
    }

    private ImageAnalysis getAnalysis() {
        ImageAnalysisConfig imageAnalysisConfig = new ImageAnalysisConfig.Builder()
                .setCallbackHandler(new Handler(handlerThread.getLooper()))
                .setLensFacing(currentFacing)
                .setImageReaderMode(ImageAnalysis.ImageReaderMode.ACQUIRE_LATEST_IMAGE)
                .setTargetResolution(new Size(640, 480))
                .build();

        ImageAnalysis imageAnalysis = new ImageAnalysis(imageAnalysisConfig);
        imageAnalysis.setAnalyzer(this);
        return imageAnalysis;
    }

    @Override
    public void onUpdated(Preview.PreviewOutput output) {
        SurfaceTexture surfaceTexture = output.getSurfaceTexture();
        if (displayer.getSurfaceTexture() != surfaceTexture) {
            if (displayer.isAvailable()) { // 处理切换摄像头
                ViewGroup parent = (ViewGroup) displayer.getParent();
                parent.removeView(displayer);
                parent.addView(displayer, 0);
                parent.requestLayout();
            }
            displayer.setSurfaceTexture(surfaceTexture);
        }
    }

    private FileOutputStream fos;

    @Override
    public void analyze(ImageProxy image, int rotationDegrees) {
        if (rtmpClient.isConnected()) {
            byte[] bytes = ImageUtils.getBytes(image, rotationDegrees, image.getWidth(), image.getHeight());
            rtmpClient.sendVideo(bytes);
        }

//        try {
//            /**
//             * 播放方法：
//             * 1、 ffplay -s 480x640 -i a.yuv    注意分辨率，因为经过了旋转，所以是480x640
//             * 2、ffmpeg -s 480x640 -i a.yuv a.mp4
//             */
//            fos.write(bytes);
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
    }

    public void toggleCamera() {
        CameraX.unbindAll();
        if (currentFacing == CameraX.LensFacing.BACK) {
            currentFacing = CameraX.LensFacing.FRONT;
        } else {
            currentFacing = CameraX.LensFacing.BACK;
        }
        CameraX.bindToLifecycle(lifeCycleOwner, getPreview(), getAnalysis());
    }

    public void release() {
        handlerThread.quitSafely();

//        try {
//            fos.close();
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
    }
}
