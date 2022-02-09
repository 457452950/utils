cmake_minimum_required(VERSION 3.10.2)
project(util)

set(CMAKE_CXX_STANDARD 17)
set(CXX_FLAG -g)

include_directories(
    include/
    include/libevent/
)
set(LIBRARY_OUTPUT_PATH lib)

add_library(logger      src/logger/Logger.cpp src/logger/LoggerBase.cpp)
add_library(asynclogger      src/logger/AsyncLogger.cpp src/logger/LoggerBase.cpp)

add_library(jsonreader  src/json_file_reader/JsonFileReader.cpp)
target_link_libraries(jsonreader logger)

add_library(redis       src/redis/Redis.cpp)
target_link_libraries(redis logger hiredis event)

add_library(tinymysql   src/mysql/libMySQL.cpp)
target_link_libraries(tinymysql logger mysqlclient)


add_library(   
    wnetwork 
    src/WNetWork/NetWorkUtils.cpp 
    src/WNetWork/WEpoll.cpp 
    src/WNetWork/WSession.cpp 
    src/WNetWork/WSingleTcpServer.cpp
    src/WBuffer.cpp 
)
target_link_libraries(wnetwork pthread)
