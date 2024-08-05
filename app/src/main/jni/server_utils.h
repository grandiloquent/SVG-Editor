#ifndef SERVER_UTILS
#define SERVER_UTILS

#include "shared.h"
#include "httplib.h"
namespace fs = std::filesystem;

void handleImagesUpload(const httplib::Request &req, httplib::Response &res);

#endif