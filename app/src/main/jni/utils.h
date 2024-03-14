#ifndef UTILS_H
#define UTILS_H

#include <filesystem>
#include "httplib.h"
#include "shared.h"


std::filesystem::path FindFile(const httplib::Request &req);

typedef struct {
    int width;
    int height;
    int degree;
} Size;


#endif