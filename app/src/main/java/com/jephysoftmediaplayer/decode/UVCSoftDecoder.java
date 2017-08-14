package com.jephysoftmediaplayer.decode;

import android.util.Log;

import java.nio.ByteBuffer;

/**
 * Created by jephy on 7/17/17.
 */
public class UVCSoftDecoder extends JNIObject{
    private final String TAG = "UVCSoftDecoder";
    private OnDecodeYUVCompeleted onDecodeYUVCompeleted;
    private byte[]  sps = {0x67,0x64,0x00,0x33,0xac-0x100,0x1b,0x1a,0x80-0x100,0x2f,0x80-0x100,0xbf-0x100,0xa1-0x100,0x00,0x00,0x03,0x00,0x01,0x00,0x00,0x03,0x00,0x3c,0x8f-0x100,0x14,0x2a,0xa0-0x100};
    private byte[] pps = {0x68,0xee-0x100,0x0b,0xcb-0x100};

    public UVCSoftDecoder(OnDecodeYUVCompeleted onDecodeYUVCompeleted) {
        this.onDecodeYUVCompeleted = onDecodeYUVCompeleted;
        construct();
        setWidth(3040);
        setHeight(1520);
        setSPS(sps,sps.length);
        setPPS(pps,pps.length);
        int ret = init("h264_mediacodec",5);
        Log.d(TAG, "UVCSoftDecoder......................初始化:" + (ret != 0 ? "失败" : "成功"));
    }

    int nativeframeCount = 0;
    long nativeTotol = 0;
    public int decode(byte[] data){
        nativeframeCount++;
        long nativeStart = System.currentTimeMillis();

        int ret = decode(data, data.length);

        long nativeCurrentTime = System.currentTimeMillis();
        nativeTotol += nativeCurrentTime - nativeStart;
        long nativeAverageTime = nativeTotol/nativeframeCount;
        Log.d(TAG, "decode帧间隔：" + nativeAverageTime);

        return ret;
    }

    public void stopDecoder(){
        destruct();
    }

    @Override
    protected void finalize() throws Throwable {
        destruct();
        super.finalize();
    }

    private void YUVPacketNoCopy(int width, int height, long timestamp, byte[] data)
    {
        ByteBuffer y = ByteBuffer.wrap(data,0,width*height);
        ByteBuffer u = ByteBuffer.wrap(data,width*height,width*height/4);
        ByteBuffer v = ByteBuffer.wrap(data,width*height + width*height/4,width*height/4);

        OnComplite(width,height,timestamp,y.array(),u.array(),v.array());
    }

    private void  OnComplite(int width, int height,long timestamp,byte[] yData,byte[] uData,byte[] vData)
    {
        onDecodeYUVCompeleted.onDecodeYUVCompeleted(yData,uData,vData,width,height,timestamp);
    }

    static {
        System.loadLibrary("media_controller-lib");
    }

    private native void construct();
    private native void destruct();

    public native int init(String name,int thread_count);
    public native int decode(byte[] data,int size);
    public native void setWidth(int width);
    public native void setHeight(int height);
    public native void setSPS(byte[] data,int size);
    public native void setPPS(byte[] data,int size);
    //专用于MP4文件,如果是H264流就不用启用
    public native void useAVCC();
    public ByteBuffer getPicture()
    {
        return null;
    }

}
