package com.example.jackyan.pptCtrl;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class ImageRecvThread extends Thread {
    private static final String TAG = "ImageRecvThread";
    private Socket sock = null;
    private InetSocketAddress address;
    private MainActivity.UIHandler uiHandler;
    ImageRecvThread(InetSocketAddress address, MainActivity.UIHandler uiHandler) {
        this.address = address;
        this.uiHandler = uiHandler;
    }
    @Override
    public void run() {
        if (sock != null && !sock.isClosed()) {
            try {
                sock.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        sock = new Socket();
        try {
            sock.connect(address);
            Log.d(TAG, "Socket is connected to " + address.toString());
            InputStream in = sock.getInputStream();
            byte[] header = new byte[4];
            int size;
            while (!isInterrupted()) {
                // read header
                size = Tools.readall(in, header, 0, 4);
                if (size == -1) {
                    break;
                }
                // read image
                int length = Tools.bytesToInt(header);
                Log.d(TAG, "length: " + length);
                byte[] buffer = new byte[length];
                size = Tools.readall(in, buffer, 0, length);
                if (size == -1) {
                    break;
                }
                // convert to bitmap
                Bitmap bitmapImage = BitmapFactory.decodeByteArray(buffer, 0, buffer.length, null);
                if (bitmapImage != null) {
                    uiHandler.setImageBitmap(bitmapRotate(bitmapImage));
                } else {
                    Log.e(TAG, "bitmap == null");
                }
            }
            graceful_close();
        } catch (IOException e) {
            e.printStackTrace();
            graceful_close();
        }
        graceful_close();
    }

    private void graceful_close() {
        try {
            sock.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private Bitmap bitmapRotate(Bitmap bitmap) {
        // 旋转图片 动作
        Matrix matrix = new Matrix();
        matrix.postRotate(0);
        // 创建新的图片
        Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0,
                bitmap.getWidth(), bitmap.getHeight(), matrix, true);
        return resizedBitmap;
    }
}
