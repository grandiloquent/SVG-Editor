cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -fno-rtti -fexceptions -Wall")

add_library(nativelib SHARED
        nativelib.cpp
        logging.cpp)

add_library(httplib SHARED httplib.cc)
#target_compile_definitions(httplib PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)

add_library(sqlite3 SHARED sqlite3.c)
add_subdirectory(nlohmann_json)
#add_subdirectory(mbedtls)
#target_include_directories(nativelib PRIVATE mbedtls/include)
add_library(shared SHARED shared.cpp)
target_link_libraries(shared android log)

add_library(server SHARED server.cpp entity.c md4c.c md4c-html.c)
add_subdirectory(zipper)
#target_include_directories(server PUBLIC zipper/zipper)
target_link_libraries(server httplib shared android sqlite3 nlohmann_json::nlohmann_json Zipper)


target_link_libraries(nativelib
        android
        server)
