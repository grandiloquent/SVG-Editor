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
    server.Post("/svg", [](const httplib::Request &req, httplib::Response &res,
                            const httplib::ContentReader &content_reader) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        nlohmann::json doc = nlohmann::json::parse(body);
        std::string title = doc["title"];
        std::string content;
        if (doc.contains("content")) {
            content = doc["content"];
        }
        if (doc.contains("id") && doc["id"] > 0) {
            int id = doc["id"];
            static const char query[]
                    = R"(UPDATE svg SET title=coalesce(?1,title),content=coalesce(?2,content),update_at=?3 where id =?4)";
            db::QueryResult fetch_row = db::query<query>(title,
                                                         content,
                                                         GetTimeStamp(),
                                                         id
            );
            res.set_content(std::to_string(fetch_row.resultCode()),
                            "text/plain; charset=UTF-8");
        } else {
            static const char query[]
                    = R"(INSERT INTO svg (title,content,create_at,update_at) VALUES(?1,?2,?3,?4))";
            db::QueryResult fetch_row = db::query<query>(title,
                                                         content,
                                                         GetTimeStamp(),
                                                         GetTimeStamp()
            );
            res.set_content(std::to_string(fetch_row.resultCode()),
                            "text/plain; charset=UTF-8");
        }
    });
    server.Get("/svg", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto id = req.get_param_value("id");
        static const char query[]
                = R"(select title, content, create_at, update_at from svg WHERE id = ?1)";
        db::QueryResult fetch_row = db::query<query>(id);
        std::string_view title, content, create_at, update_at;

        if (fetch_row(title, content,create_at, update_at)) {
            nlohmann::json j = {
                    {"title",     title},
                    {"content",   content},
                    {"create_at", create_at},
                    {"update_at", update_at},
            };
            res.set_content(j.dump(), "application/json");
        }
    });
    server.listen(host, port);
}