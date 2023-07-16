cmake_minimum_required(VERSION 3.10.2)
project(util)
MESSAGE("operation system is ${CMAKE_HOST_SYSTEM_NAME}")

if (WIN32)
    message("not support windows!!!")
    return()
endif ()

if (APPLE)
    message("not support APPLE!!!")
    return()
endif ()

if (UNIX)
    message("Unix!!!")

    if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")

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


        add_library(jsonreader src/json/JsonFileReader.cpp include/wutils/network/base/ISocket.h)
        target_link_libraries(jsonreader hiredis)


        add_library(
                wutils
                src/ByteArray.cpp
                src/Error.cpp

                src/threadpool/ThreadPool.cpp

                src/network/Channel.cpp

                src/buffer/ChainBuffer.cpp
                src/buffer/RingBuffer.cpp
                src/buffer/StraightBuffer.cpp

                include/wutils/base/Defer.h
                include/wutils/base/HeadOnly.h

                include/wutils/network/base/ISocket.h
                include/wutils/network/base/Definition.h
                include/wutils/network/easy/Udp.h
                include/wutils/network/easy/Tcp.h
                include/wutils/network/easy/Timer.h
                include/wutils/network/io_event/EpollContext.h
                include/wutils/network/io_event/SelectContext.h
                include/wutils/network/Timer.h
                include/wutils/network/Acceptor.h
        )

        add_subdirectory(demo)

    elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Unix")
        message("not support Unix!!!")
        return()
    endif ()

endif ()
