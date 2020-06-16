#include <jni.h>
#include <string>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#include <malloc.h>

#define LOG_TAG "louis"
#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
#define LOGD(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef  struct GifBean{
    int current_frame;
    int total_frame;
    int *dealys;
}GifBean;

void drawFrame(GifFileType *pType, GifBean *pBean, AndroidBitmapInfo info, void *pVoid);

extern "C"
JNIEXPORT jlong JNICALL
Java_com_louis_gif_GifHandler_loadPath(JNIEnv *env, jobject thiz, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    int err;
    GifFileType *gifFileType =  DGifOpenFileName(path,&err);
    DGifSlurp(gifFileType);

    // gif  帧数 总帧数 每一帧播放的时间
    GifBean *gifBean = (GifBean *) malloc(sizeof(GifBean));
    //清空内存地址
    memset(gifBean, 0, sizeof(GifBean));
    gifFileType->UserData = gifBean;
    gifBean->dealys = (int *) malloc(sizeof(int) * gifFileType->ImageCount);
    memset(gifBean->dealys, 0, sizeof(int) * gifFileType->ImageCount);
    gifBean->total_frame = gifFileType->ImageCount;
    gifBean->current_frame = 0;

    //遍历每一帧 图形控制扩展快

    for (int i = 0; i < gifBean->total_frame; ++i) {
        SavedImage frame = gifFileType->SavedImages[i];
        ExtensionBlock * extensionBlock;
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                extensionBlock = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if (extensionBlock) {
            //延迟时间 两个字节表示一个int
            int dealy = (10) * (extensionBlock->Bytes[2] << 8 | extensionBlock->Bytes[1]);
            LOGD("时间 %d ",dealy);
            gifBean->dealys[i] = dealy;

        }
    }

//    env->ReleaseStringUTFChars(path_, path);
    return (jlong)gifFileType;
}

void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    GifImageDesc frameInfo = savedImage.ImageDesc;
    //整幅bitmap的首地址
    int *px = (int *) pixels;
    //每一行的首地址
    int *line;
    //用来解压缩的字典
    ColorMapObject *colorMapObject = frameInfo.ColorMap;
    px = (int *) ((char *) px + info.stride * frameInfo.Top);

    GifByteType gifByteType;
    GifColorType gifColorType;
    //先遍历行
    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        line = px;
        //遍历列
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
            //索引
            int pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            //当前帧的像素数据
            gifByteType = savedImage.RasterBits[pointPixel];
            //字典
            gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        //换行
        px = (int*)((char*)px + info.stride);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_louis_gif_GifHandler_getWidth(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = (GifFileType *)(ndk_gif);
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_louis_gif_GifHandler_getHeight(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    return gifFileType->SHeight;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_louis_gif_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong ndk_gif,
                                          jobject bitmap) {
    GifFileType *gifFileType= (GifFileType *)ndk_gif;
    GifBean *gifBean = (GifBean *) gifFileType->UserData;

    //绘制
    AndroidBitmapInfo info;
    //uint32_t stride:每一行的像素
    AndroidBitmap_getInfo(env, bitmap, &info);
    //bitmap 转换缓冲区
    void  *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    //绘制
    drawFrame(gifFileType, gifBean, info, pixels);
    //
    gifBean->current_frame += 1;
    LOGD("当前帧 %d ",gifBean->current_frame);
    if (gifBean->current_frame >= gifBean->total_frame - 1) {
        gifBean->current_frame = 0;
    }
    AndroidBitmap_unlockPixels(env, bitmap);

    return gifBean->dealys[gifBean->current_frame];
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_louis_gif_GifHandler_release(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    // TODO: implement release()
}