package com.jephysoftmediaplayer.util;

import android.os.Environment;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by jephy on 7/22/17.
 */

public class FrameWriter {

    private final static String TAG = "FrameWriter";

    public final File defaultDir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/UVCResource");

    private int fileNum = 0;//文件命名计数

    /**
     * 默认初始化帧数据存放路径
     */
    public FrameWriter()
    {
    }

    /**
     * 将ByteBuffer数据写入到存储卡的特定目录
     * @param byteBuffer 需要写入的数据
     * @param destDir 写入的目录
     * @return
     */
    public boolean writeFrameToStorage(ByteBuffer byteBuffer, File destDir){
        return writeFrameToStorage(byteBuffer.array(),destDir);
    }

    /**
     * 将data数据写入到存储卡的特定目录
     * @param data 需要写入的数据
     * @param destDir 写入的目录
     * @return
     */
    public boolean writeFrameToStorage(byte[] data,File destDir){

        Log.d(TAG, "writeFile....数组长度。。。" + data.length);
        if (null == destDir){
            destDir = defaultDir;
        }

        if (!destDir.exists()) {
            destDir.mkdirs();
        }

        File file = new File(destDir, "/" + fileNum + "_frame.txt");
        FileOutputStream outputStream = null;
        BufferedOutputStream bufferedOutputStream=null;
        try {
            outputStream = new FileOutputStream(file);
            bufferedOutputStream = new BufferedOutputStream(outputStream);
//            byte[] bytes = new byte[frame.remaining()];
//            frame.get(bytes);
//            outputStream.write(bytes);
            bufferedOutputStream.write(data);
            bufferedOutputStream.flush();
            fileNum++;
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }finally {
            try {
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return false;
    }
}
