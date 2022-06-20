cmake_minimum_required(VERSION 3.10.2)
project(util)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall ")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -fsanitize=address")

include_directories(
        include/
        include/libevent/
)
set(LIBRARY_OUTPUT_PATH lib)

if(WIN32)
        message("not support windows!!!")
        return()
endif()


add_library(redis src/redis/Redis.cpp)
target_link_libraries(redis hiredis event)

add_library(tinymysql src/mysql/libMySQL.cpp)
target_link_libraries(tinymysql mysqlclient)



add_library(jsonreader src/json_file_reader/JsonFileReader.cpp)
target_link_libraries(jsonreader)


add_library(
        wutils
        src/WSystem.cpp
        src/WBuffer.cpp
        src/thread_pool/WThreadPool.cpp
        
        include/WNetWork/WEpoll.h
        include/WNetWork/WChannel.h
        include/WNetWork/WNetWorkUtils.h
        include/WNetWork/WSelect.h
        include/WNetWork/WNetWork.h
        include/WNetWork/WEvent.h
        include/WNetWork/WSingleTcpServer.h
        src/WNetWork/NetWorkUtils.cpp
        src/WNetWork/WEpoll.cpp
        src/WNetWork/WChannel.cpp
        src/WNetWork/WSelect.cpp
        src/WNetWork/WEvent.cpp
        src/WNetWork/WSingleTcpServer.cpp
)
target_link_libraries(wutils pthread)

add_library(logger src/logger/AsyncLogger.cpp src/logger/LoggerBase.cpp)
target_link_libraries(logger wutils)

add_subdirectory(demo)


