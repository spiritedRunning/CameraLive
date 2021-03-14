package com.example.cameralive;

import android.Manifest;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;

import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraX;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageAnalysisConfig;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.core.PreviewConfig;
import androidx.core.app.ActivityCompat;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class MainActivity extends AppCompatActivity implements Preview.OnPreviewOutputUpdateListener, ImageAnalysis.Analyzer {

    private TextureView textureView;
    private HandlerThread handlerThread;

    private CameraX.LensFacing currentFacing = CameraX.LensFacing.BACK;

    private FileOutputStream fos;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textureView = findViewById(R.id.textureView);
        handlerThread = new HandlerThread("Analysis-Thread");
        handlerThread.start();

        CameraX.bindToLifecycle(this, getPreview(), getAnalysis());

        ActivityCompat.requestPermissions(this, new String[]{
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.CAMERA
        }, 100);

        try {
            fos = new FileOutputStream("/sdcard/a.yuv");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
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
        if (textureView.getSurfaceTexture() != surfaceTexture) {
            if (textureView.isAvailable()) { // 处理切换摄像头
                ViewGroup parent = (ViewGroup) textureView.getParent();
                parent.removeView(textureView);
                parent.addView(textureView, 0);
                parent.requestLayout();
            }
            textureView.setSurfaceTexture(surfaceTexture);
        }
    }


    @Override
    public void analyze(ImageProxy image, int rotationDegrees) {

        byte[] bytes = ImageUtils.getBytes(image, rotationDegrees, image.getWidth(), image.getHeight());

        try {
            /**
             * 播放方法：
             * 1、 ffplay -s 480x640 -i a.yuv    注意分辨率，因为经过了旋转，所以是480x640
             * 2、ffmpeg -s 480x640 -i a.yuv a.mp4
             */
            fos.write(bytes);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void toggleCamera(View view) {
        CameraX.unbindAll();
        if (currentFacing == CameraX.LensFacing.BACK) {
            currentFacing = CameraX.LensFacing.FRONT;
        } else {
            currentFacing = CameraX.LensFacing.BACK;
        }
        CameraX.bindToLifecycle(this, getPreview(), getAnalysis());
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();

        handlerThread.quitSafely();
        try {
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


}
