cmake_minimum_required(VERSION 3.10.2)
project(util)
MESSAGE("operation system is ${CMAKE_HOST_SYSTEM_NAME}")

if (WIN32)
    message("not support windows!!!")
    add_definitions(-D OS_IS_WINDOWS)
    set(WINDOWS 1)

    add_subdirectory(demo)

    return()
endif ()

if (APPLE)
    message("not support APPLE!!!")
    return()
endif ()

if (UNIX)
    message("Unix!!!")

    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
        message("Linux!!!")
        add_definitions(-D OS_IS_LINUX)
        set(LINUX 1)

        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall ")
        #        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -fsanitize=address")

        include_directories(
                include/
                include/libevent/
        )
        set(LIBRARY_OUTPUT_PATH lib)


        add_library(redis src/redis/Redis.cpp)
        target_link_libraries(redis hiredis event)

        add_library(tinymysql src/mysql/MySQLClient.cpp)
        target_link_libraries(tinymysql mysqlclient)


        add_library(jsonreader src/json/JsonFileReader.cpp)
        target_link_libraries(jsonreader hiredis)


        add_library(
                wutils
                src/System.cpp
                src/ByteArray.cpp
                src/Error.cpp

                src/threadpool/ThreadPool.cpp

                src/network/NetWorkUtils.cpp
                src/network/Epoll.cpp
                src/network/Channel.cpp
                src/network/SingleTcpServer.cpp

                src/logger/AsyncLogger.cpp
                src/logger/LoggerBase.cpp

                src/buffer/ChainBuffer.cpp
                src/buffer/RingBuffer.cpp
                src/buffer/StraightBuffer.cpp

                src/ssl/md5.cpp
        )
        #        target_link_libraries(wutils pthread)

        add_library(logger src/logger/AsyncLogger.cpp src/logger/LoggerBase.cpp)
        target_link_libraries(logger wutils)

        add_subdirectory(demo)

    elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Unix")
        message("not support Unix!!!")
        return()
    endif ()

endif ()
