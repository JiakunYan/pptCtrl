package com.example.jackyan.pptCtrl;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.InetSocketAddress;
import java.net.Socket;

public class MessageClientService extends Service {
    private InetSocketAddress server_address = null;
    private Socket sock = null;
    private boolean isConnected = false;
    private BufferedWriter writer = null;

    public MessageClientService() {
    }

    // connect to server_address
    // block method
    public boolean connect_block(InetSocketAddress server_address) {
        this.server_address = server_address;
        Thread connect_thread = new Thread(new Connect_async());
        connect_thread.start();
        // wait for connect
        try {
            connect_thread.join();
        } catch(InterruptedException e) {
            e.printStackTrace();
        }
        return isConnected;
    }

    public boolean connect_status(){
        return isConnected;
    }

    // connect to this.server_address
    class Connect_async implements Runnable {
        @Override
        public void run() {
            isConnected = connect_sync();
        }
    }

    // send message nonblocking
    public void sendMessage(String message) {
        new Thread(new Send(message)).start();
    }

    class Send implements Runnable {
        private String message;
        Send(String message) {
            this.message = message;
        }

        @Override
        public void run() {
            if (!isConnected) {
                Log.e("MessageClientService", "Socket is not connected! Try connecting...");
                boolean succ = connect_sync();
                if (!succ) {
                    isConnected = false;
                    return;
                }
                isConnected = true;
            }
            try {
                writer.write(message + '\n');
                writer.flush();
                Log.i("MessageClientService","Send succeed! message = " + message);
            } catch (IOException e) {
                e.printStackTrace();
                isConnected = false;
            }
        }
    }


    // terminal operation
    public void stop() {
        if(sock != null && !sock.isClosed()){
            try {
                sock.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            sock = null;
            Log.i("MessageClientService", "Socket is closed");
        }
        isConnected = false;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return new MyBinder();
    }

    class MyBinder extends Binder {
        MessageClientService getService(){
            return MessageClientService.this;
        }
    }

    @Override
    public void onDestroy() {
        stop();
        super.onDestroy();
    }

    // terminal operation
    // sock connect to address and set writer
    // guarantee if return true
    private boolean connect_sync() {
        stop();
        sock = new Socket();
        if (server_address == null) {
            Log.e("MessageClientService","server_address is null!");
            return false;
        }
        try {
            sock.connect(server_address, 1000);
            writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            Log.i("MessageClientService", "Socket is connected to " + server_address.toString());
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }
}
