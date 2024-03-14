#include "server.h"

static const char db_name[] = "/storage/emulated/0/.editor/svg.db";
using db = sqlite::Database<db_name>;

void StartServer(JNIEnv *env, jobject assetManager, const std::string &host, int port) {
    static const char table[]
            = R"(CREATE TABLE IF NOT EXISTS "svg" (
	"id"	INTEGER NOT NULL UNIQUE,
	"title"	TEXT NOT NULL,
	"content"	TEXT NOT NULL,
	"create_at"	INTEGER,
	"update_at"	INTEGER,
	PRIMARY KEY("id" AUTOINCREMENT)
))";
    db::query<table>();
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    std::map <std::string, std::string> t{};
    std::string d{"application/octet-stream"};
    httplib::Server server;
    server.Get(R"(/(.+\.(?:js|css|html|xhtml|ttf|png|jpg|jpeg|gif|json|svg|wasm))?)",
               [&t, mgr, &d](const httplib::Request &req,
                             httplib::Response &res) {
                   res.set_header("Access-Control-Allow-Origin", "*");
                   auto p = req.path == "/" ? "index.html" : req.path.substr(1);
                   unsigned char *data;
                   unsigned int len = 0;
                   ReadBytesAsset(mgr, p,
                                  &data, &len);
                   auto str = std::string(reinterpret_cast<const char *>(data), len);

                   auto content_type = httplib::detail::find_content_type(p, t, d);
                   res.set_content(str, content_type);
               });
    server.listen(host, port);
}