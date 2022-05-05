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

add_library(logger src/logger/AsyncLogger.cpp src/logger/LoggerBase.cpp)

add_library(jsonreader src/json_file_reader/JsonFileReader.cpp)
target_link_libraries(jsonreader)

add_library(redis src/redis/Redis.cpp)
target_link_libraries(redis hiredis event)

add_library(tinymysql src/mysql/libMySQL.cpp)
target_link_libraries(tinymysql mysqlclient)

add_library(
        wutils
        src/WSystem.cpp
        src/WBuffer.cpp
        src/WDebugger.cpp
        src/WTimer.cpp
        src/thread_pool/WThreadPool.cpp
)
target_link_libraries(wutils pthread)

add_library(
        wnetwork
        include/WNetWork/WBaseSession.hpp
        include/WNetWork/WConnection.hpp
        include/WNetWork/WEpoll.hpp
        include/WNetWork/WMultiTcpServer.hpp
        include/WNetWork/WNetWork.h
        include/WNetWork/WNetWorkHandler.hpp
        include/WNetWork/WNetWorkUtils.h
        include/WNetWork/WService.hpp
        include/WNetWork/WSingleTcpServer.hpp
        include/WNetWork/WTcpClient.hpp
        include/WNetWork/WTimerHandler.hpp
        src/WNetWork/NetWorkUtils.cpp
        src/WNetWork/WBaseSession.cpp
        src/WNetWork/WConnection.cpp
        src/WNetWork/WEpoll.cpp
        src/WNetWork/WMultiTcpServer.cpp
        src/WNetWork/WSingleTcpServer.cpp
        src/WNetWork/WTcpClient.cpp
)
target_link_libraries(wnetwork pthread wutils)


