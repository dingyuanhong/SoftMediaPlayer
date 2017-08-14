//
// Created by JunfeiYang on 17/7/11.
//

#include "jni.h"
#include "string"
#include <assert.h>
#include "Android/log.h"

#include "time.h"
static JavaVM* g_VM = NULL;

class JNIObject
{
public:
    JNIObject()
    :javaVM(NULL),env_(NULL),thiz(NULL)
    {
    }
    ~JNIObject()
    {
        assert(thiz == NULL);
    }

    void attachParent(JavaVM * vm,JNIEnv *env,jobject object)
    {
        javaVM =vm;
        thiz = env->NewGlobalRef(object);
    }

    void detachParent(JNIEnv *env)
    {
        if(thiz != NULL) {
            env->DeleteGlobalRef(thiz);
            thiz = NULL;
        }
    }

private:
    void AttachThread()
    {
        if(javaVM == NULL) return;

        int ret = javaVM->AttachCurrentThread(&env_,NULL);
        if(ret != 0)
        {
        }

    }

    void DetachThread()
    {
        if(javaVM == NULL) return;
        javaVM->DetachCurrentThread();
    }

protected:
    JavaVM* javaVM;
    JNIEnv *env_;
    jobject thiz;
};

#define  CONSTRUCT(T) { T *t = new T(); \
    jclass clazz = (jclass)(*env).GetObjectClass(thiz); \
    jfieldID fid = (jfieldID)(*env).GetFieldID(clazz, "mObj", "J"); \
    jlong jstr = (jlong) (*env).GetLongField(thiz, fid);  \
    (*env).SetLongField(thiz, fid, (jlong)t);}

#define OBJECT(T,name) jclass clazz = (jclass)env->GetObjectClass(thiz); \
     jfieldID fid = env->GetFieldID(clazz, "mObj","J");  \
     T *name = (T *)env->GetLongField(thiz, fid);

#define DESTRUCT(T)  jclass clazz = (jclass)env->GetObjectClass(thiz); \
     jfieldID fid = env->GetFieldID(clazz, "mObj","J");  \
     T *object = (T *)env->GetLongField(thiz, fid); \
     if(object != NULL) delete object; \
     (*env).SetLongField(thiz, fid, (jlong)0);

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);
    g_VM = vm;
    return JNI_VERSION_1_4;
}

#include "MediaDecode.h"
#include "EvoInterface/sei_packet.h"

class JNIMediaDecode
        : public JNIObject,
        public MediaDecode
{
public:
    JNIMediaDecode()
    :curEnv_(NULL)
    {
        YUVData_ = NULL;
    }
    void SetCurrentEnv(JNIEnv * env)
    {
        curEnv_ = env;
    }
protected:
    virtual int CreateFrame(AVFrame *out,int Width, int Height, AVPixelFormat Format)
    {
        JNIEnv *env = curEnv_;
        if(YUVData_ == NULL && env != NULL)
        {
            YUVData_ = env->NewByteArray(Width*Height*3/2);
        }
        if(YUVData_ == NULL)
        {
            __android_log_print(ANDROID_LOG_ERROR,"native JNIMediaDecode","NewByteArray Error.");
            return 0;
        }
        jbyte * YUVData = env->GetByteArrayElements(YUVData_,0);
        uint8_t * src_buffer = (uint8_t*)YUVData;
        int ret = av_image_fill_arrays(out->data, out->linesize,src_buffer,Format,Width,Height,1);
        return ret;
    }

    virtual void FreeFrame(AVFrame **out)
    {
        JNIEnv *env = curEnv_;
        if (YUVData_ != NULL && env != NULL)
        {
            env->DeleteLocalRef(YUVData_);
            YUVData_ = NULL;
        }
        if(YUVData_ != NULL)
        {
            __android_log_print(ANDROID_LOG_ERROR,"native JNIMediaDecode","FreeFrame Error.");
        }
        YUVData_ = NULL;
        if (out == NULL) return;
        AVFrame *frame = *out;
        av_frame_free(out);
        *out = NULL;
    }
private:
    void YUVPacketNoCopy(int width,int height,int64_t timestamp,uint8_t* data,int size)
    {
        JNIEnv *env = curEnv_;
        if(env == NULL) return;

        jclass clazz = env->GetObjectClass(thiz);
        jmethodID  jid = env->GetMethodID(clazz,"YUVPacketNoCopy","(IIJ[B)V");
        env->CallVoidMethod(thiz,jid,width,height,timestamp,YUVData_);
        if (YUVData_ != NULL && env != NULL)
        {
            env->DeleteLocalRef(YUVData_);
            YUVData_ = NULL;
        }
        __android_log_print(ANDROID_LOG_DEBUG,"native JNIMediaDecode","YUVPacketNoCopy");

    }

    void SendPacket(AVFrame * frame)
    {
        YUVPacketNoCopy(frame->width,frame->height,frame->pts,frame->data[0],frame->pkt_size);
    }
private:
    JNIEnv * curEnv_;
    jbyteArray YUVData_;
};

extern "C"
JNIEXPORT jobject JNICALL
Java_com_jephysoftmediaplayer_decode_VideoStamp_Analysis(JNIEnv *env, jclass clazz,
                                                            jbyteArray data_, jint size) {
    jobject obj = env->AllocObject(clazz);

    jbyte *data = env->GetByteArrayElements(data_, NULL);
    jfieldID pts = env->GetFieldID(clazz, "pts","J");
    jfieldID dts = env->GetFieldID(clazz, "dts","J");
    jfieldID timestamp = env->GetFieldID(clazz, "timestamp","J");
    jfieldID flag = env->GetFieldID(clazz, "flag","I");
    jfieldID timebase_num = env->GetFieldID(clazz, "timebase_num","I");
    jfieldID timebase_den = env->GetFieldID(clazz, "timebase_den","I");

    char buffer[256];
    int count = 256;
    int ret = get_sei_content((unsigned char *)data,size,buffer,&count);

    env->ReleaseByteArrayElements(data_, data, 0);

    if(ret > 0)
    {
        int cflags = 0;
        int64_t cpts = 0;
        int64_t cdts = 0;
        int64_t ctimestamp = 0;
        int ctime_base_num = 0;
        int ctime_base_den = 0;

        sscanf(buffer,"flags:%d pts:%llu dts:%llu timestamp:%llu time_base:num:%d den:%d",
        &cflags,&cpts,&cdts,&ctimestamp,&ctime_base_num,&ctime_base_den);

        env->SetLongField(obj,pts,cpts );
        env->SetLongField(obj,dts,cdts);
        env->SetLongField(obj,timestamp,ctimestamp);
        env->SetIntField(obj,flag,cflags );
        env->SetIntField(obj,timebase_num,ctime_base_num);
        env->SetIntField(obj,timebase_den,ctime_base_den);
    } else
    {
        env->DeleteLocalRef(obj);
        obj = NULL;
    }
    return obj;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_construct(JNIEnv* env, jobject thiz){
    CONSTRUCT(JNIMediaDecode);
    OBJECT(JNIMediaDecode,control);
    if(control == NULL) return ;
    control->attachParent(g_VM,env,thiz);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_destruct(JNIEnv *env,
                                                                     jobject thiz) {
    {
        OBJECT(JNIMediaDecode, control);
        if (control == NULL) return;
        control->detachParent(env);
    }
    DESTRUCT(JNIMediaDecode);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_init(JNIEnv *env, jobject thiz,
                                                         jstring name_, jint thread_count) {
    const char *name = env->GetStringUTFChars(name_, 0);
    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return -1;
    av_jni_set_java_vm(g_VM,NULL);
    int ret = control->init(name,thread_count);
    env->ReleaseStringUTFChars(name_, name);
    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_setWidth(JNIEnv *env, jobject thiz,
                                                             jint width) {
    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return ;
    return control->SetWidth(width);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_setHeight(JNIEnv *env, jobject thiz,
                                                              jint height) {
    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return ;
    return control->SetHeight(height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_setSPS(JNIEnv *env, jobject thiz,
                                                           jbyteArray data_, jint size) {
    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return ;
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    control->SetSPS((uint8_t*)data,size);
    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_setPPS(JNIEnv *env, jobject thiz,
                                                           jbyteArray data_, jint size) {
    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return ;
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    control->SetPPS((uint8_t*)data,size);
    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_useAVCC(JNIEnv *env, jobject thiz) {

    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return ;
    control->UseAVCC();
}

int64_t count = 0;
long totle_time = 0;

extern "C"
JNIEXPORT jint JNICALL
Java_com_jephysoftmediaplayer_decode_UVCSoftDecoder_decode(JNIEnv *env,
                                                           jobject thiz,
                                                                 jbyteArray data,
                                                             jint size)
{
    OBJECT(JNIMediaDecode, control);
    if (control == NULL) return -1;
    control->SetCurrentEnv(env);
    int ret = 0;
    jbyte* byte =env->GetByteArrayElements(data, 0);
    __android_log_print(ANDROID_LOG_DEBUG,"native media_controller","begin");
    if(size > 0 && byte != NULL)
    {
        count++;
        int64_t timeBegin = av_gettime()/1000;
        ret = control->decode((uint8_t*)byte,size);
        int64_t timeEnd = av_gettime() / 1000;
        totle_time += timeEnd-timeBegin;
        __android_log_print(ANDROID_LOG_DEBUG, "native media_controller", "use:%lld\n", totle_time/count);
    } else
    {
        ret = control->decode((uint8_t*)NULL,0);
    }
    __android_log_print(ANDROID_LOG_DEBUG,"native media_controller","end");
    control->SetCurrentEnv(NULL);
    return ret;
}