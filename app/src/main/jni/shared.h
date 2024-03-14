#ifndef SHARED_H
#define SHARED_H

#include <string>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include  <assert.h>

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "B5aOx2::", __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, "B5aOx2::", __VA_ARGS__))

int GetTimeStamp();

bool ReadBytesAsset(AAssetManager *aAssetManager, std::string_view filename, unsigned char **data,
                    unsigned int *len);

std::string ReadPreference(JNIEnv *env, jobject context, jmethodID getString, const char *key);

std::string Substring(const std::string &s, const std::string &start, const std::string &end);

std::string SubstringAfterLast(const std::string &s, const std::string &start);

std::string SubstringBeforeLast(const std::string &s, const std::string &start);

std::string GetLocalIp();
std::string EncodeUrl(const std::string &s);
void ReplaceAll(std::string& in_out, const std::string& search, const std::string& substitute);
std::string ReplaceFirst(std::string str, std::string_view token, std::string_view to);
std::vector<std::string> ReadAllLines(const std::string &filepath);
std::string ReadAllText(const std::string& filepath);
#endif
/*
add_library(shared SHARED shared.cpp)
#include "shared.h";
*/