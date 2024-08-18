#ifndef SERVER_UTILS
#define SERVER_UTILS

#include "shared.h"
#include "httplib.h"
#include "key.h"
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

void handleImagesUpload(const httplib::Request &req, httplib::Response &res);
std::string Trans(const std::string &q, const std::string &to);
void handleGemini(const httplib::Request &req, httplib::Response &res);
std::string uploadImage(std::string &content, std::string ilename);
#endif