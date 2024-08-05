#include "server_utils.h"


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

void handleImagesUpload(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    const auto &image_file = req.get_file_value("images");
    auto id = req.has_param("id") ? req.get_param_value("id") : "1";
    auto imageFileName = getUniqueFileName(id, image_file.filename);
    std::ofstream ofs(imageFileName, std::ios::binary);
    ofs << image_file.content;
    res.set_content(SubstringAfterLast(imageFileName, "images/"), "text/plain");
}