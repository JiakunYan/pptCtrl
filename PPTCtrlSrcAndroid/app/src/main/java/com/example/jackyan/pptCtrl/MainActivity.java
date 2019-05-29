package com.example.jackyan.pptCtrl;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.media.AudioManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import java.net.InetSocketAddress;

public class MainActivity extends AppCompatActivity {
    static final String TAG = "MainActivity";
    UIHandler uiHandler;
    ImageRecvThread mImageRecvThread;
    VolumeKeyMonitor mVolumeKeyMonitor = null;
    private ImageView backgroundImageView;
    private Touchpad touchpad;
    private ImageButton cfg_button,button_before,button_next;
    private RelativeLayout control_pad;
    private LinearLayout config_pad;
    //private Switch video_stream_switch;
    private int control_method = -1;
    private Pair<Float, Float> prev;
    private Pair<Float, Float> prev2;
    private MessageClientService messageClientService = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // bind service
        setService();

        initView();
        uiHandler = new UIHandler();

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        mVolumeKeyMonitor = new VolumeKeyMonitor();
        mVolumeKeyMonitor.start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mImageRecvThread != null && mImageRecvThread.isAlive()) {
            mImageRecvThread.interrupt();
        }
        if (mVolumeKeyMonitor != null && mVolumeKeyMonitor.isAlive()) {
            mVolumeKeyMonitor.interrupt();
        }
    }

    private void setService(){
        ServiceConnection conn=new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                MessageClientService.MyBinder myBinder = (MessageClientService.MyBinder) service;
                messageClientService = myBinder.getService();
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                messageClientService = null;
            }
        };

        Intent intent = new Intent(this, MessageClientService.class);

        bindService(intent, conn, Context.BIND_AUTO_CREATE);
    }

    private void initView(){
        backgroundImageView= findViewById(R.id.background_view);
        touchpad= findViewById(R.id.touchPad);                //super class，解决触屏事件
        cfg_button= findViewById(R.id.button_config);   //选择，弹出主控制界面
        config_pad= findViewById(R.id.cfg_pad);      //主控制界面，ip端口等信息
        control_pad= findViewById(R.id.control_pad);         //在界面右侧选择控制方式的界面
        button_before= findViewById(R.id.button_before);
        button_next= findViewById(R.id.button_next);
        Button button_exit= findViewById(R.id.button_exit);

        final ImageView[] method_images = new ImageView[4];
        for(int i=0;i<4;++i){
            //初始化每个Method的图像，然后用监听并确定选择了那个method
            int resId = getResources().getIdentifier("m0" + (i + 1), "id", getPackageName());
            method_images[i] = findViewById(resId);
            method_images[i].setOnClickListener(new View.OnClickListener(){
                @Override
                public void onClick(View v){
                    pack_layouts();
                    for(int j=0;j<4;++j){
                        if(v.getId()==method_images[j].getId()){
                            int drawable_id = getResources().getIdentifier
                                    ("n0" + (j + 1), "drawable", getPackageName());
                            control_method=j;
                            method_images[j].setImageResource(drawable_id);
                            messageClientService.sendMessage("E");
                        }else{
                            int drawable_id = getResources().getIdentifier
                                    ("m0" + (j + 1), "drawable", getPackageName());
                            method_images[j].setImageResource(drawable_id);
                        }
                    }
                    unpack_layout(control_method);
                }
            });
        }

        config_pad.setVisibility(View.GONE);
        touchpad.setVisibility(View.GONE);

        backgroundImageView.setKeepScreenOn(true);
        backgroundImageView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.hide();
        }

        //打开主控制界面键
        cfg_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cfg_button.setVisibility(View.GONE);
                config_pad.setVisibility(View.VISIBLE);
                pack_layouts();  //所有模式的控制布局收起
                control_pad.setVisibility(View.GONE);
            }
        });

        button_exit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cfg_button.setVisibility(View.VISIBLE);
                config_pad.setVisibility(View.GONE);
                control_pad.setVisibility(View.VISIBLE);
            }
        });

        //模式0：翻页（default）
        button_before.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage("l");
            }
        });

        button_next.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage("r");
            }
        });

        //模式1、2、3：触屏事件,返回点.1：高光；2：放大；3：涂鸦。
        touchpad.setOnTouchListener(new View.OnTouchListener() {
            /*
            If this period(from ACTION_DOWN to ACTION_UP) entered dual mode one time, disable single mode.
            Avoid simultaneously up your fingers but the rect move away.
             */
            private boolean dual_mode = false;
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent){
                int pointerCount = motionEvent.getPointerCount();
                if (pointerCount > 2) {
                    pointerCount = 2;
                }
                int actionType = motionEvent.getActionMasked();
                switch (actionType) {
                    case MotionEvent.ACTION_DOWN: {   //初次触摸屏幕时触发
                        if (control_method == 3) {
                            messageClientService.sendMessage("e");
                        }
                        Pair<Float, Float> current = new Pair<>(motionEvent.getX(), motionEvent.getY());
                        sendPoint(current);
                        prev = current;
                        dual_mode = false;
                        break;
                    }
                    case MotionEvent.ACTION_POINTER_DOWN: {//第二个点被触摸时触发
                        Log.d(TAG, "ACTION_POINTER_DOWN");
                        Pair<Float, Float> current1 = new Pair<>(motionEvent.getX(0), motionEvent.getY(0));
                        Pair<Float, Float> current2 = new Pair<>(motionEvent.getX(1), motionEvent.getY(1));
                        sendPoints(current1, current2);
                        prev = current1;
                        prev2 = current2;
                        dual_mode = true;
                        break;
                    }
                    case MotionEvent.ACTION_MOVE:
                        if (pointerCount == 1) {
                            if (!dual_mode && distance(motionEvent.getX(0), motionEvent.getY(0), prev.first, prev.second) > 20 && control_method != 2) {
                                Pair<Float, Float> current = new Pair<>(motionEvent.getX(), motionEvent.getY());
                                sendPoint(current);
                                prev = current;
                            }
                        } else if (pointerCount > 1) {
                            if (dual_mode && distance(motionEvent.getX(0), motionEvent.getY(0), prev.first, prev.second) > 20 || distance(motionEvent.getX(1), motionEvent.getY(1), prev2.first, prev2.second) > 20 && control_method != 2) {
                                Log.d(TAG, "ACTION_MOVE, two point");
                                Pair<Float, Float> current1 = new Pair<>(motionEvent.getX(0), motionEvent.getY(0));
                                Pair<Float, Float> current2 = new Pair<>(motionEvent.getX(1), motionEvent.getY(1));
                                sendPoints(current1, current2);
                                prev = current1;
                                prev2 = current2;
                            }
                        }
                        break;
                    case MotionEvent.ACTION_POINTER_UP:
                        Log.d(TAG, "ACTION_POINTER_UP");
                        Pair<Float, Float> current1 = new Pair<>(motionEvent.getX(0), motionEvent.getY(0));
                        Pair<Float, Float> current2 = new Pair<>(motionEvent.getX(1), motionEvent.getY(1));
                        sendPoints(current1, current2);
                        prev = current1;
                        prev2 = current2;
                        break;
                    case MotionEvent.ACTION_UP:     //手离开屏幕的动作
                        if (control_method == 3) {
                            messageClientService.sendMessage("e");
                        } else if (!dual_mode) {
                            Pair<Float, Float> current = new Pair<>(motionEvent.getX(), motionEvent.getY());
                            sendPoint(current);
                            prev = current;
                        }
                        break;
                }
                return true;
            }
        });

        //收起所有模式控制布局
        pack_layouts();
    }

    private double distance(double x1, double y1, double x2, double y2) {
        double ac = Math.abs(y2 - y1);
        double cb = Math.abs(x2 - x1);
        return Math.hypot(ac, cb);
    }

    private void sendPoint(Pair<Float,Float> thispoint){
        if(messageClientService.connect_status()){
            try{
                if(control_method==1) {
                    messageClientService.sendMessage("h," + thispoint.first.intValue()+","+ thispoint.second.intValue()); //高光功能
                } else if(control_method==2){
                    messageClientService.sendMessage("m," + thispoint.first.intValue() +","+ thispoint.second.intValue());  //画线功能
                } else if(control_method==3){
                    messageClientService.sendMessage("d," + thispoint.first.intValue() +","+ thispoint.second.intValue());  //画线功能
                }
            }catch (Exception e){
                e.printStackTrace();
            }
        }
    }

    private void sendPoints(Pair<Float,Float> point1, Pair<Float,Float> point2){
        if(messageClientService.connect_status()){
            try{
                if(control_method==1) {
                    messageClientService.sendMessage("h2," + point1.first.intValue()+","
                            + point1.second.intValue()+"," + point2.first.intValue()+","+ point2.second.intValue()); //高光功能
                } else if(control_method==2){
                    messageClientService.sendMessage("m2," + point1.first.intValue()+","
                            + point1.second.intValue()+"," + point2.first.intValue()+","+ point2.second.intValue());  //画线功能
                }
            }catch (Exception e){
                e.printStackTrace();
            }
        }
    }

    public void onConnectClick(View view) {
        // get address from addressInput
        TextView addressView = findViewById(R.id.ipView);
        EditText ipInput = findViewById(R.id.ipInput);
        String ipNum = ipInput.getText().toString();
        EditText portInput = findViewById(R.id.clientPortInput);
        int portNum;
        try {
            portNum = Integer.parseInt(portInput.getText().toString());
        } catch (NumberFormatException e) {
            addressView.setText("Wrong format of PortNum");
            return;
        }
        InetSocketAddress address = null;
        try {
            address = new InetSocketAddress(ipNum, portNum);
        } catch (Exception e) {
            addressView.setText(String.format("Cannot connect to %s:%d", ipNum, portNum));
            return;
        }
        // connect
        if (messageClientService != null) {
            //和指定的客户端连接
            if (messageClientService.connect_block(address)) {
                if (mImageRecvThread != null && mImageRecvThread.isAlive()) {
                    mImageRecvThread.interrupt();
                }
                mImageRecvThread = new ImageRecvThread(address, uiHandler);
                mImageRecvThread.start();
                // set UI
                addressView.setText(String.format("Connected to %s:%d", ipNum, portNum));
            }
            else {
                // set UI
                addressView.setText(String.format("Cannot connect to %s:%d", ipNum, portNum));
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode,KeyEvent event) {
        switch (keyCode){
            case KeyEvent.KEYCODE_VOLUME_UP:
                sendMessage("l");
                return true;
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                sendMessage("r");
                return true;
            default:
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    public void onStopClick(View view) {
        System.out.println("Video off");
        if (messageClientService != null) {
            messageClientService.stop();
        }
        if (mImageRecvThread != null && mImageRecvThread.isAlive()) {
            mImageRecvThread.interrupt();
            try {
                mImageRecvThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        uiHandler.setImageDrawable(R.drawable.theme);
        // set UI
        TextView addressView = findViewById(R.id.ipView);
        addressView.setText("Disconnected");
    }

    //传送指令信息
    private void sendMessage(final String s){
        new Thread(new Runnable() {
            @Override
            public void run() {
                if(messageClientService.connect_status()) {
                    messageClientService.sendMessage(s);
                }
            }
        }
        ).start();
    }

    //处理IP地址
    private String intToIp(int i){
        return (i&0xFF)+"."+((i>>8)&0xFF)+"."+((i>>16)&0xFF)+"."+((i>>24)&0xFF);
    }

    //控制方法的界面展开
    private void unpack_layout(int control_method){
        switch(control_method){
            case 0:
                button_before.setVisibility(View.VISIBLE);
                button_next.setVisibility(View.VISIBLE);
                break;
            case 1:
                touchpad.setVisibility(View.VISIBLE);
                break;
            case 2:
                touchpad.setVisibility(View.VISIBLE);
                break;
            case 3:
                touchpad.setVisibility(View.VISIBLE);
                break;
            default:
                break;
        }
    }

    //收起所有控制布局，（翻页、高亮范围程度等）
    private void pack_layouts(){
        for(int i=0;i<3;++i)
            pack_layout(i);
    }

    private void pack_layout(int control_method){
        switch (control_method){
            case 0:
                button_next.setVisibility(View.GONE);
                button_before.setVisibility(View.GONE);
                break;
            case 1:
                touchpad.setVisibility(View.GONE);
                break;
            case 2:
                touchpad.setVisibility(View.GONE);
                break;
            default:
                break;
        }
    }

    public class VolumeKeyMonitor extends Thread {
        private AudioManager mAudioManager;
        private int defaultVolume;

        VolumeKeyMonitor() {
            // 获得AudioManager对象
            mAudioManager = (AudioManager) getApplicationContext().getSystemService(Context.AUDIO_SERVICE);//音乐音量,如果要监听铃声音量变化，则改为AudioManager.STREAM_RING
            // calculate default volume
            int maxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
            int minVolume = mAudioManager.getStreamMinVolume(AudioManager.STREAM_MUSIC);
            int currentVolume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
            if (currentVolume == minVolume) {
                defaultVolume = currentVolume + 1;
            } else if (currentVolume == maxVolume) {
                defaultVolume = currentVolume - 1;
            } else {
                defaultVolume = currentVolume;
            }
        }

        public void run() {
            try {
                while (!Thread.interrupted()) {
                    int currentVolume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
                    Log.i("Volume","currentVolume = " + currentVolume);
                    if (defaultVolume < currentVolume) {
                        // volume increase
                        onVolumeUp();
                        mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, defaultVolume,
                                AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
                    } else if (defaultVolume > currentVolume) {
                        // volume decrease
                        onVolumeDown();
                        mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, defaultVolume,
                                AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
                    }
                    Thread.sleep(100);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        void onVolumeUp() {
            sendMessage("l");
        }

        void onVolumeDown() {
            sendMessage("r");
        }
    }

    class UIHandler extends Handler {
        public static final int SET_BITMAP = 0;
        public static final int SET_DRAWABLE = 1;
        private ImageView mImageView;
        UIHandler() {
            mImageView = findViewById(R.id.background_view);
        }
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SET_BITMAP:
                    mImageView.setImageBitmap((Bitmap)msg.obj);
                    break;
                case SET_DRAWABLE:
                    mImageView.setImageResource(msg.arg1);
                    break;
            }
        }
        public void setImageBitmap(Bitmap mBitmap) {
            Message msg = new Message();
            msg.what = SET_BITMAP;
            msg.obj = mBitmap;
            this.sendMessage(msg);
        }
        public void setImageDrawable(int resId) {
            Message msg = new Message();
            msg.what = SET_DRAWABLE;
            msg.arg1 = resId;
            this.sendMessage(msg);
        }
    }
}



