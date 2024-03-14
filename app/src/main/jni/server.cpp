#include "server.h"

static const char db_name[] = "/storage/emulated/0/.editor/svg.db";
using db = sqlite::Database<db_name>;

void StartServer(JNIEnv *env, jobject assetManager, const std::string &host, int port) {

}