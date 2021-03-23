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

public class MainActivity extends AppCompatActivity {

    private TextureView textureView;
    private RtmpClient rtmpClient;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textureView = findViewById(R.id.textureView);

        ActivityCompat.requestPermissions(this, new String[]{
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.CAMERA
        }, 100);


        rtmpClient = new RtmpClient(this);
        rtmpClient.initVideo(textureView, 480, 640, 10, 640_000);
    }


    public void toggleCamera(View view) {
        rtmpClient.toggleCamera();
    }

    public void startLive(View view) {
        rtmpClient.startLive("rtmp://");
    }

    public void stopLive(View view) {
        rtmpClient.stopLive();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        rtmpClient.release();
    }


}
