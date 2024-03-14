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
    httplib::Server server;
    server.listen(host, port);
}