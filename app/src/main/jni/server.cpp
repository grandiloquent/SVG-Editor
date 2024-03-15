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
    static const char table1[]
            = R"(CREATE TABLE IF NOT EXISTS "snippet" (
	"id"	INTEGER,
	"keyword" TEXT,
	"content"	TEXT,
	"views" INTEGER,
	"type" TEXT,
	PRIMARY KEY("id" AUTOINCREMENT)
))";
    db::query<table1>();
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    std::map<std::string, std::string> t{};
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
            static const char query1[]
                    = R"(SELECT last_insert_rowid())";
            std::string_view rowid;
            auto fr = db::query<query1>();
            if (fr(rowid)) {
                res.set_content(std::string{rowid.begin(), rowid.end()},
                                "text/plain; charset=UTF-8");
            } else {
                res.set_content(std::to_string(fetch_row.resultCode()),
                                "text/plain; charset=UTF-8");
            }

        }
    });
    server.Get("/svg", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto id = req.get_param_value("id");
        static const char query[]
                = R"(select title, content, create_at, update_at from svg WHERE id = ?1)";
        db::QueryResult fetch_row = db::query<query>(id);
        std::string_view title, content, create_at, update_at;

        if (fetch_row(title, content, create_at, update_at)) {
            nlohmann::json j = {
                    {"title",     title},
                    {"content",   content},
                    {"create_at", create_at},
                    {"update_at", update_at},
            };
            res.set_content(j.dump(), "application/json");
        }
    });
    server.Get("/svgs", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        static const char query[]
                = R"(SELECT id,title,update_at FROM svg ORDER BY update_at DESC)";
        db::QueryResult fetch_row = db::query<query>();
        std::string_view id, title, update_at;

        nlohmann::json doc = nlohmann::json::array();
        while (fetch_row(id, title, update_at)) {
            nlohmann::json j = {

                    {"id",        id},
                    {"title",     title},
                    {"update_at", update_at},

            };
            doc.push_back(j);
        }
        res.set_content(doc.dump(), "application/json");
    });
    server.Get("/snippets", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto t = req.get_param_value("t");
        static const char query[]
                = R"(SELECT content,keyword,id FROM snippet where type = ?1 ORDER BY views)";
        db::QueryResult fetch_row = db::query<query>(t);
        std::string_view content, keyword, id;
        nlohmann::json doc = nlohmann::json::array();
        while (fetch_row(content, keyword, id)) {
            nlohmann::json j = {
                    {"content", content},
                    {"keyword", keyword},
                    {"id",      id}
            };
            doc.push_back(j);
        }
        res.set_content(doc.dump(), "application/json");
    });
    server.Get("/snippet", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto k = req.get_param_value("k");
        auto t = req.get_param_value("t");

        static const char query[]
                = R"(SELECT content FROM snippet where keyword = ?1 and type = ?2)";
        db::QueryResult fetch_row = db::query<query>(k, t);
        std::string_view content;
        if (fetch_row(content)) {
            res.set_content(content.data(), content.size(), "application/json");
        }

    });

    server.Post("/snippet", [](const httplib::Request &req, httplib::Response &res,
                               const httplib::ContentReader &content_reader) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        nlohmann::json doc = nlohmann::json::parse(body);
        std::string keyword;
        if (doc.contains("keyword")) {
            keyword = doc["keyword"];
        }
        std::string content;
        if (doc.contains("content")) {
            content = doc["content"];
        }
        std::string type;
        if (doc.contains("type")) {
            type = doc["type"];
        }
        int id = 0;
        if (doc.contains("id")) {
            id = doc["id"];
        }
        if (id == 0) {
            static const char query[]
                    = R"(INSERT INTO snippet (content,keyword,type) VALUES(?1,?2,?3))";
            db::QueryResult fetch_row = db::query<query>(content, keyword, type);

            res.set_content(std::to_string(fetch_row.resultCode()),
                            "text/plain; charset=UTF-8");
        } else {
            static const char query[]
                    = R"(UPDATE snippet SET content=coalesce(NULLIF(?1,''),content),keyword=coalesce(NULLIF(?2,''),keyword),type=coalesce(NULLIF(?3,''),type) where id =?4)";
            db::QueryResult fetch_row = db::query<query>(content, keyword, type, id);

            res.set_content(std::to_string(fetch_row.resultCode()),
                            "text/plain; charset=UTF-8");
        }

    });

    server.Post("/snippet/hit", [](const httplib::Request &req, httplib::Response &res,
                                   const httplib::ContentReader &content_reader) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        static const char query[]
                = R"(UPDATE snippet SET views = COALESCE(views,0) + 1 WHERE content = ?1)";
        db::QueryResult fetch_row = db::query<query>(body);

        res.set_content(std::to_string(fetch_row.resultCode()),
                        "text/plain; charset=UTF-8");
    });
    server.Get("/svgviewer", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto id = req.get_param_value("id");
        static const char query[]
                = R"(select title, content, create_at, update_at from svg WHERE id = ?1)";
        db::QueryResult fetch_row = db::query<query>(id);
        std::string_view title, content, create_at, update_at;

        if (fetch_row(title, content, create_at, update_at)) {

            std::stringstream ss;
            ss << R"(<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" << title << R"(</title>
</head>

<body>)" << content << R"(</body></html>)";
            res.set_content(ss.str(), "text/html");
        }
    });
    server.listen(host, port);
}