package com.louis.gif;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private ImageView imageView;
    private GifHandler gifHandler;
    private Bitmap bitmap;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        imageView = findViewById(R.id.image);
    }

    public void ndkLoadGif(View view) {
        File file = new File(Environment.getExternalStorageDirectory(), "demo.gif");
        gifHandler = new GifHandler(file.getAbsolutePath());

        bitmap = Bitmap.createBitmap(gifHandler.getWidth(), gifHandler.getHeight(),Bitmap.Config.ARGB_8888);

        int delay = gifHandler.updateFrame(bitmap);

        handler.sendEmptyMessageDelayed(1, delay);
    }

    Handler handler = new Handler(){
        @Override
        public void handleMessage(@NonNull Message msg) {
            int delay = gifHandler.updateFrame(bitmap);
            imageView.setImageBitmap(bitmap);
            handler.sendEmptyMessageDelayed(1, delay);
        }
    };
}
