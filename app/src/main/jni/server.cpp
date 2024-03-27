#include "server.h"

static const char db_name[] = "/storage/emulated/0/.editor/svg.db";
using db = sqlite::Database<db_name>;

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

void serveFile(const std::filesystem::path &p, httplib::Response &res,
               const std::map<std::string, std::string> t, std::string &d) {

    if (p.extension().string() == ".html" || p.extension().string() == ".xhtml") {
        auto s = ReadAllText(p);
        s = ReplaceFirst(s, "</head>",
                         R"(<meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"></head>)");
        res.set_content(s, "text/html");
        return;
    }

    std::shared_ptr<std::ifstream> fs = std::make_shared<std::ifstream>();
    fs->exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        fs->open(p, std::ios_base::binary);
    } catch (std::system_error &e) {
        res.status = 404;
        return;
    }
    fs->seekg(0, std::ios_base::end);
    auto end = fs->tellg();

    if (end == 0)return;
    fs->seekg(0);
    std::map<std::string, std::string> file_extension_and_mimetype_map;
    auto contentType = httplib::detail::find_content_type(p, t, d);
    if (contentType.empty()) {
        contentType = "application/octet-stream";
    }
    res.set_content_provider(static_cast<size_t>(end),
                             contentType,
                             [fs](uint64_t offset,
                                  uint64_t length,
                                  httplib::DataSink &sink) {
                                 if (fs->fail()) {
                                     return false;
                                 }
                                 fs->seekg(offset, std::ios_base::beg);
                                 size_t bufSize = 81920;
                                 char buffer[bufSize];

                                 try {
                                     fs->read(buffer, bufSize);
                                 } catch (std::system_error &e) {
                                 }
                                 sink.write(buffer,
                                            static_cast<size_t>(fs->gcount()));
                                 return true;
                             });
}


std::filesystem::path FindFile(const httplib::Request &req) {

    auto dir = std::filesystem::path{"/storage/emulated/0/.editor"};
    auto p = dir.append(req.path.substr(1));
    if (exists(p)) {
        return p;
    }
    return std::filesystem::path{
    };
}

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

    static const char table2[]
            = R"(CREATE TABLE IF NOT EXISTS "tag" (
	"id"	INTEGER,
	"name" TEXT NOT NULL UNIQUE,
	PRIMARY KEY("id" AUTOINCREMENT)
))";
    db::query<table2>();
    static const char table3[]
            = R"(CREATE TABLE IF NOT EXISTS "svg_tag" (
	"id"	INTEGER,
	"svg_id"	INTEGER,
	"tag_id"	INTEGER,
	PRIMARY KEY("id" AUTOINCREMENT)
))";
    db::query<table3>();

    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    std::map<std::string, std::string> t{};
    std::string d{"application/octet-stream"};
    httplib::Server server;
    server.Get(
            R"(/(.+\.(?:js|css|html|xhtml|ttf|png|jpg|jpeg|gif|json|svg|wasm|babylon|blend|glb|ogg))?)",
            [&t, mgr, &d](const httplib::Request &req,
                          httplib::Response &res) {
                res.set_header("Access-Control-Allow-Origin", "*");
                auto p = req.path == "/" ? "index.html" : req.path.substr(1);
                unsigned char *data;
                unsigned int len = 0;
                ReadBytesAsset(mgr, p,
                               &data, &len);
                auto str = std::string(reinterpret_cast<const char *>(data), len);
                if (str.length() == 0) {
                    auto file = FindFile(req);
                    if (is_regular_file(file)) {
                        serveFile(file, res, t, d);
                        return;
                    }
                }
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
    server.Get("/search", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        auto q = req.get_param_value("q");
        static const char query[]
                = R"(SELECT id,title,content,update_at FROM svg)";
        db::QueryResult fetch_row = db::query<query>();
        std::string id, title, content, update_at;

        nlohmann::json doc = nlohmann::json::array();
        std::regex q_regex(q);
        while (fetch_row(id, title, content, update_at)) {
            if (std::regex_search(title, q_regex) || std::regex_search(content, q_regex)) {
                nlohmann::json j = {

                        {"id",        id},
                        {"title",     title},
                        {"update_at", update_at},

                };
                doc.push_back(j);
            }

        }
        res.set_content(doc.dump(), "application/json");
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
            if (content.find("const createScene = ") != std::string::npos) {

                ss << R"(<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" << title << R"(</title>
</head>

<body>
<script src="https://cdnjs.cloudflare.com/ajax/libs/dat-gui/0.6.2/dat.gui.min.js"></script>
<script src="https://assets.babylonjs.com/generated/Assets.js"></script>
<script src="https://cdn.babylonjs.com/recast.js"></script>
<script src="https://cdn.babylonjs.com/ammo.js"></script>
<script src="https://cdn.babylonjs.com/havok/HavokPhysics_umd.js"></script>
<script src="https://cdn.babylonjs.com/cannon.js"></script>
<script src="https://cdn.babylonjs.com/Oimo.js"></script>
<script src="https://cdn.babylonjs.com/earcut.min.js"></script>
<script src="https://cdn.babylonjs.com/babylon.js"></script>
<script src="https://cdn.babylonjs.com/materialsLibrary/babylonjs.materials.min.js"></script>
<script src="https://cdn.babylonjs.com/proceduralTexturesLibrary/babylonjs.proceduralTextures.min.js"></script>
<script src="https://cdn.babylonjs.com/postProcessesLibrary/babylonjs.postProcess.min.js"></script>
<script src="https://cdn.babylonjs.com/loaders/babylonjs.loaders.js"></script>
<script src="https://cdn.babylonjs.com/serializers/babylonjs.serializers.min.js"></script>
<script src="https://cdn.babylonjs.com/gui/babylon.gui.min.js"></script>
<script src="https://cdn.babylonjs.com/inspector/babylon.inspector.bundle.js"></script>

<style>
  html,
  body {
    overflow: hidden;
    width: 100%;
    height: 100%;
    margin: 0;
    padding: 0;
  }

  #renderCanvas {
    width: 100%;
    height: 100%;
    touch-action: none;
  }

  #canvasZone {
    width: 100%;
    height: 100%;
  }
</style>
<script>
  window.onerror = function(errMsg, url, line, column, error) {
    var result = !column ? '' : 'column: ' + column;
    result += !error;

    const div = document.createElement("div");
    div.textContent = "\nError= " + errMsg + "\nurl= " + url + "\nline= " + line + result;
    document.body.appendChild(div);
    var suppressErrorAlert = true;
    return suppressErrorAlert;
  };
  console.error = function(e) {
    document.body.innerHTML = "<pre>" + [...arguments].map(x => `${JSON.stringify(x).replaceAll(/\\n/g,"\n")}`).join('\n') + "</pre>";
  }
</script>
<div id="canvasZone"><canvas id="renderCanvas"></canvas></div>
<script type="module">

//let baseUri = window.location.host === "127.0.0.1:5500" ? "http://192.168.8.55:8500" : "";



  var canvas = document.getElementById("renderCanvas");

  var startRenderLoop = function(engine, canvas) {
    engine.runRenderLoop(function() {
      if (sceneToRender && sceneToRender.activeCamera) {
        sceneToRender.render();
      }
    });
  }

  var engine = null;
  var scene = null;
  var sceneToRender = null;
  var createDefaultEngine = function() {
    return new BABYLON.Engine(canvas, true, {
      preserveDrawingBuffer: true,
      stencil: true,
      disableWebGL2Support: false
    });
  };
)" << content << R"(
window.initFunction = async function() {



    var asyncEngineCreation = async function() {
      try {
        return createDefaultEngine();
      } catch (e) {
        console.log("the available createEngine function failed. Creating the default engine instead");
        return createDefaultEngine();
      }
    }

// window.
    engine = await asyncEngineCreation();
    if (!engine) throw 'engine should not be null.';
    startRenderLoop(engine, canvas);
//   window.
  scene =await createScene();
  };
  initFunction().then(() => {
    sceneToRender = scene
  });

  // Resize
  window.addEventListener("resize", function() {
    engine.resize();
  });
</script></body></html>)";

            } else if (content.find("type=\"x-shader/x-fragment\"") != std::string::npos) {
                ss << R"(<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" << title << R"(</title>
</head>

<body>
<script id="vs" type="x-shader/x-vertex">#version 300 es
in vec4 a_position;
     void main() {
       gl_Position = a_position;
     }
     </script>
)" << content << R"(</body></html>)";
            } else {
                ss << R"(<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)" << title << R"(</title>
</head>

<body>)" << content << R"(</body></html>)";
            }

            res.set_content(ss.str(), "text/html");
        }
    });
    server.Get("/trans", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto q = req.get_param_value("q");
        auto to = req.get_param_value("to");
        auto s = Trans(q, to);
        res.set_content(s, "application/json");
    });
    server.Post("/svgtag", [](const httplib::Request &req, httplib::Response &res,
                              const httplib::ContentReader &content_reader) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string body;
        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
            return true;
        });
        nlohmann::json doc = nlohmann::json::parse(body);

        int id = 0;
        if (doc.contains("id")) {
            id = doc["id"];
        }
        if (id == 0) {
            res.status = 404;
            return;
        }
        std::vector<std::string> names = doc["names"];
        if (names.empty()) {
            res.status = 404;
            return;
        }
        static const char query[]
                = R"(delete from svg_tag where svg_id =?1)";
        db::query<query>(id);
        for (auto i: names) {
            static const char q1[]
                    = R"(select id from tag where name =?1)";
            static const char q2[]
                    = R"(insert into svg_tag(svg_id,tag_id) values(?1,?2))";

            static const char q3[]
                    = R"(insert into tag(name) values(?1))";
            db::QueryResult fetch_row = db::query<q1>(i);
            std::string tag_id;
            if (!fetch_row(tag_id)) {
                db::query<q3>(i);
            }
            fetch_row = db::query<q1>(i);
            if (fetch_row(tag_id)) {
                db::query<q2>(std::to_string(id), tag_id);
            }
        }


    });
    server.Get("/svgtags", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        auto id = req.get_param_value("id");
        if (id.empty()) {
            static const char query[]
                    = R"(select name FROM tag)";
            db::QueryResult fetch_row = db::query<query>();
            std::string_view name;
            nlohmann::json doc = nlohmann::json::array();
            while (fetch_row(name)) {
                doc.push_back(name);
            }
            res.set_content(doc.dump(), "application/json");
            return;
        } else {
//            static const char q1[]
//                    = R"(delete FROM tag where id<>164)";
//            db::query<q1>();
            static const char query[]
                    = R"(select name FROM tag JOIN svg_tag on tag.id = svg_tag.tag_id where svg_tag.svg_id=$1)";
            db::QueryResult fetch_row = db::query<query>(id);
            std::string_view name;
            nlohmann::json doc = nlohmann::json::array();
            while (fetch_row(name)) {
                doc.push_back(name);
            }
            res.set_content(doc.dump(), "application/json");
        }

    });

    server.listen(host, port);
}