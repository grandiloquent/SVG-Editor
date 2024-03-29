#ifndef SERVER_H
#define SERVER_H

#include "shared.h"
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <stdio.h>
#include <filesystem>
#include <fstream>
#include "SQLiteWrapper.h"
namespace fs = std::filesystem;
void StartServer(JNIEnv *env, jobject assetManager, const std::string &host, int port);

#endif