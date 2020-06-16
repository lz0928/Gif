package com.louis.gif;


import android.graphics.Bitmap;

public class GifHandler {

    private long gifAddr;

    public GifHandler(String path) {
        gifAddr = loadPath(path);
    }

    public int getWidth(){
        return getWidth(gifAddr);
    }

    public int getHeight(){
        return getHeight(gifAddr);
    }

    public int updateFrame(Bitmap bitmap) {
        return updateFrame(gifAddr,bitmap);
    }

    private native long loadPath(String path);

    public native int getWidth(long ndkGif);
    public native int getHeight(long ndkGif);

    public native int updateFrame(long ndkGif, Bitmap bitmap);

    public native int release(long ndkGif);

}
