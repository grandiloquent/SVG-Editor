#include "server_utils.h"


std::string buildFileName(const std::string &dir, int index, const std::string &extension) {
    std::ostringstream ss;
    ss << dir << "/" << index << "." << extension;
    return ss.str();
}

void handleImagesUpload(const httplib::Request &req, httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    const auto &image_file = req.get_file_value("images");
    auto id = req.has_param("id") ? req.get_param_value("id") : "1";
    auto dir = "/storage/emulated/0/.editor/images/" + id;
    if (!fs::is_directory(dir))
        fs::create_directory(dir);
    auto extension = SubstringAfterLast(image_file.filename, ".");
    int count = 1;
    auto image = buildFileName(dir, count, extension);

    while (fs::exists(image)) {
        count++;
        image = buildFileName(dir, count, extension);
    }
    std::ofstream ofs(image, std::ios::binary);
    ofs << image_file.content;
    res.set_content(SubstringAfterLast(image, "images/"), "text/plain");
}