#include "utils.h"


std::filesystem::path FindFile(const httplib::Request &req) {
    auto path = req.get_param_value("path");
    auto file = std::filesystem::path{path};
    if (exists(file)) {
        return file;
    }
    if (!req.get_header_value("Referer").empty()) {
        auto referer = req.get_header_value("Referer");
        std::filesystem::path d(
                SubstringAfterLast(httplib::detail::decode_url(referer, true), "="));

        auto p = d.parent_path();
        p = p.append(req.path.substr(1));
        if (exists(p)) {
            return p;
        }
        auto dir = d.parent_path().parent_path();
        p = dir.append(req.path.substr(1));

        if (exists(p)) {
            return p;
        }
        dir = std::filesystem::path{"/storage/emulated/0/.editor"};
        p = dir.append(req.path.substr(1));
        if (exists(p)) {
            return p;
        }

    }
    return std::filesystem::path{};
}