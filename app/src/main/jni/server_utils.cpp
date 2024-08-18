#include "server_utils.h"


std::string Trans(const std::string &q, const std::string &to) {
    httplib::Client cli("translate.google.com", 80);
    std::stringstream ss;
    ss << "/translate_a/single?client=gtx&sl=auto&tl=";
    ss << to;
    ss << "&dt=t&dt=bd&ie=UTF-8&oe=UTF-8&dj=1&source=icon&q=";
    ss << EncodeUrl(q);
    if (auto res = cli.Get(
            ss.str(),
            {{"User-Agent",
              "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
              "(KHTML, like Gecko) Chrome/104.0.0.0 Safari/537.36"}})) {

        return res->body;
    } else {
        return {};
    }
}

std::string buildFileName(const std::string &dir, int index, const std::string &extension) {
    std::ostringstream ss;
    ss << dir << "/" << index << "." << extension;
    return ss.str();
}

std::string getUniqueFileName(const std::string &id, const std::string &originalFileName) {
    auto dir = "/storage/emulated/0/.editor/images/" + id;
    if (!fs::is_directory(dir))
        fs::create_directory(dir);
    auto extension = SubstringAfterLast(originalFileName, ".");
    int count = 1;
    auto image = buildFileName(dir, count, extension);
    while (fs::exists(image)) {
        count++;
        image = buildFileName(dir, count, extension);
    }
    return image;
}

void saveLocal(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    const auto &image_file = req.get_file_value("images");
    auto id = req.has_param("id") ? req.get_param_value("id") : "1";
    auto imageFileName = getUniqueFileName(id, image_file.filename);
    std::ofstream ofs(imageFileName, std::ios::binary);
    ofs << image_file.content;
    res.set_content(SubstringAfterLast(imageFileName, "images/"), "text/plain");
}

void handleImagesUpload(const httplib::Request &req, httplib::Response &res) {
    const auto &image_file = req.get_file_value("images");
    // std::stringstream buffer;
    // buffer << image_file.content;
    httplib::MultipartFormDataItems items = {
            {"images", image_file.content, image_file.filename, "application/octet-stream"},
    };
    //httplib::Client cli("192.168.8.190", 8000);
    httplib::SSLClient cli("chenyunyoga.cn", 443);
    cli.enable_server_certificate_verification(false);
    auto result = cli.Post("/upload", items);
    if (result) {
        res.set_content(result->body, "text/plain");
        return;
    }
    res.status = 404;
}

std::string uploadImage(std::string &content, std::string filename) {
    httplib::MultipartFormDataItems items = {
            {"images", content, filename, "application/octet-stream"},
    };
    //httplib::Client cli("192.168.8.190", 8000);
    httplib::SSLClient cli("chenyunyoga.cn", 443);
    cli.enable_server_certificate_verification(false);
    auto result = cli.Post("/upload", items);
    if (result) {
        return result->body;
    }
    return std::string{};
}

void handleGemini(const httplib::Request &req, httplib::Response &res) {

    auto q = req.get_param_value("q");
    //httplib::Client cli("192.168.8.190", 8000);
    httplib::SSLClient cli("generativelanguage.googleapis.com", 443);
    cli.enable_server_certificate_verification(false);
    // /v1beta/models/gemini-pro:generateContent?key=
    nlohmann::json j;
    j["contents"][0]["parts"][0]["text"] = q;
    auto result = cli.Post(KEY, {}, j.dump(), "application/json");
    if (result) {
        res.set_content(result->body, "text/plain");
        return;
    }
    res.status = 404;
}