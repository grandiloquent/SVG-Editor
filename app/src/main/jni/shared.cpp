#include <regex>
#include <fstream>
#include "shared.h"


int GetTimeStamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

bool ReadBytesAsset(AAssetManager *aAssetManager, std::string_view filename, unsigned char **data,
                    unsigned int *len) {
    AAsset *aAsset = AAssetManager_open(aAssetManager, filename.data(), AASSET_MODE_BUFFER);
    if (aAsset == nullptr) {
        *data = nullptr;
        if (len) *len = 0;
        return false;
    }
    auto size = (unsigned int) AAsset_getLength(aAsset);
    *data = (unsigned char *) malloc(size);
    AAsset_read(aAsset, *data, size);
    if (len) *len = size;
    AAsset_close(aAsset);
    return true;
}

std::string ReadPreference(JNIEnv *env, jobject context, jmethodID getString, const char *key) {
    jstring jKey = env->NewStringUTF(key);
    auto jValue = env->CallObjectMethod(context, getString,
                                        jKey);
    const char *cValue = env->GetStringUTFChars(static_cast<jstring>(jValue),
                                                nullptr);
    std::string value(cValue);
    env->ReleaseStringUTFChars(static_cast<jstring>(jValue), cValue);
    return value;
}

std::string Substring(const std::string &s, const std::string &start, const std::string &end) {
    auto startPos = s.find(start);
    if (startPos == std::string::npos) {
        return s;
    }
    auto length = start.length();
    auto endPos = s.find(end, startPos + length);
    if (endPos == std::string::npos) {
        return s;
    }
    return s.substr(startPos + length, endPos - (startPos + length));
}

std::string SubstringAfterLast(const std::string &s, const std::string &start) {
    auto startPos = s.rfind(start);
    if (startPos == std::string::npos) {
        return s;
    }
    return s.substr(startPos + start.length());
}

std::string SubstringBeforeLast(const std::string &s, const std::string &start) {
    auto startPos = s.rfind(start);
    if (startPos == std::string::npos) {
        return s;
    }
    return s.substr(0, startPos);
}

std::string GetLocalIp() {
    std::string ipAddress = "0.0.0.0";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while (temp_addr != NULL) {
            if (temp_addr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if (!strcmp(temp_addr->ifa_name, "wlan0") || !strcmp(temp_addr->ifa_name, "en0")) {
                    ipAddress = inet_ntoa(((struct sockaddr_in *) temp_addr->ifa_addr)->sin_addr);
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return ipAddress;
}
std::string EncodeUrl(const std::string &s) {
    std::string result;
    result.reserve(s.size());

    for (size_t i = 0; s[i]; i++) {
        switch (s[i]) {
            case ' ':
                result += "%20";
                break;
            case '+':
                result += "%2B";
                break;
            case '\r':
                result += "%0D";
                break;
            case '\n':
                result += "%0A";
                break;
            case '\'':
                result += "%27";
                break;
            case '&':
                result += "%26";
                break;
            case ',':
                result += "%2C";
                break;
                // case ':': result += "%3A"; break; // ok? probably...
            case ';':
                result += "%3B";
                break;
            case '/':
                result += "%2F";
                break;
            case '%':
                result += "%25";
                break;

            default:
                auto c = static_cast<uint8_t>(s[i]);
                if (c >= 0x80) {
                    result += '%';
                    char hex[4];
                    auto len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                    assert(len == 2);
                    result.append(hex, static_cast<size_t>(len));
                } else {
                    result += s[i];
                }
                break;
        }
    }

    return result;
}

void ReplaceAll(std::string& in_out, const std::string& search, const std::string& substitute)
{
    if (search.empty()) return;
    in_out = std::regex_replace(in_out, std::regex(search), substitute);
}
std::string ReplaceFirst(std::string str, std::string_view token, std::string_view to) {
    const auto startPos = str.find(token);
    if (startPos == std::string::npos)
        return str;

    str.replace(startPos, token.length(), to);
    return str;
}

std::vector<std::string> ReadAllLines(const std::string& filepath)
{
    std::ifstream file(filepath);
    std::string str;
    std::vector<std::string> lines;

    while (std::getline(file, str))
    {
        lines.push_back(str);
    }

    if(file.is_open())
        file.close();
    return lines;
}
std::string ReadAllText(const std::string& filepath)
{
    std::ifstream file(filepath);
    std::string str;
    std::string file_contents;

    while (std::getline(file, str))
    {
        file_contents += str;
        file_contents.push_back('\n');
    }

    if(file.is_open())
        file.close();
    return file_contents;
}

