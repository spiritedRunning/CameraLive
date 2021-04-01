package com.example.cameralive;

import android.Manifest;
import android.os.Bundle;
import android.view.TextureView;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

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
                Manifest.permission.CAMERA,
                Manifest.permission.RECORD_AUDIO,
        }, 100);


        rtmpClient = new RtmpClient(this);
        rtmpClient.initVideo(textureView, 480, 640, 10, 640_000);
        rtmpClient.initAudio(44100, 2);
    }


    public void toggleCamera(View view) {
        rtmpClient.toggleCamera();
    }

    public void startLive(View view) {
        rtmpClient.startLive("rtmp://192.168.1.103/rtmplive/android");
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
