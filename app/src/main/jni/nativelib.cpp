#include <jni.h>

#include "java_interop.h"
#include "server.h"

extern "C" jstring
Java_psycho_euphoria_svg_MainActivity_startServer(JNIEnv *env, jclass obj, jobject context,
                                                  jobject assetManager,
                                                  jstring ip,
                                                  jint port) {
    const std::string host = jsonparse::jni::Convert<std::string>::from(env, ip);
    StartServer(env, assetManager, host, port);

    char msg[60] = "Hello ";
    jstring result;
    result = env->NewStringUTF(msg);
    return result;
}
