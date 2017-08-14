package com.jephysoftmediaplayer;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;

//import com.evomotion.glrenderview.GlVideoRenderLayout;
import com.jephysoftmediaplayer.player.JephyPlayer;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class PlayerTestActivity extends Activity  {

    private final String TAG = "PlayerTestActivity";
    private static final int DECODED_SUCCESS = 0;

    static
    {
//        System.loadLibrary("stagefright");
//        System.loadLibrary("avcodec-56");
//        System.loadLibrary("avfilter-5");
//        System.loadLibrary("avformat-56");
//        System.loadLibrary("avutil-54");
//        System.loadLibrary("swresample-1");
//        System.loadLibrary("swscale-3");
    }

//    @BindView(R.id.vidoe_render_layout)
//    GlVideoRenderLayout videoRenderLayout;

    @BindView(R.id.start_play_bt)
    Button startPlayButton;

    @BindView(R.id.stop_play_bt)
    Button stopPlayButton;
    private JephyPlayer player;

    @BindView(R.id.pause_play_bt)
    Button pausePlayButton;

    @OnClick(R.id.start_play_bt)
    void startPlay(Button button) {
        Log.d(TAG, "click start button");
        player.start();
    }

    @OnClick(R.id.pause_play_bt)
    void pausePlay(){
        Log.d(TAG, "click pause button");
        player.pause();
    }

    @OnClick(R.id.stop_play_bt)
    void stopPlay(Button button) {

        Log.d(TAG, "click stop button");
        player.stop();

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mock_data);
        ButterKnife.bind(this);

        player = new JephyPlayer();
//        player.setDisplay(videoRenderLayout);

        player.prepare();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }
}
