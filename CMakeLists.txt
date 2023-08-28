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

        add_library(
                wutils

                src/threadpool/ThreadPool.cpp

                src/buffer/ChainBuffer.cpp
                src/buffer/RingBuffer.cpp
                src/buffer/StraightBuffer.cpp

                include/wutils/base/Defer.h
                include/wutils/base/HeadOnly.h

                include/wutils/network/base/ISocket.h
                include/wutils/network/base/Definition.h
                include/wutils/network/base/Native.h

                include/wutils/network/easy/Udp.h
                include/wutils/network/easy/Tcp.h
                include/wutils/network/easy/Timer.h
                include/wutils/network/easy/Acceptor.h
                include/wutils/network/easy/Epoll.h
                include/wutils/network/easy/Select.h
                include/wutils/network/easy/Tools.h
                include/wutils/network/easy/Event.h

                include/wutils/network/io_event/IOEvent.h
                include/wutils/network/io_event/IOContext.h
                include/wutils/network/io_event/EpollContext.h
                include/wutils/network/io_event/SelectContext.h

                include/wutils/network/NetAddress.h
                include/wutils/network/Factory.h
                include/wutils/network/Network.h
                include/wutils/network/Timer.h
                include/wutils/network/Acceptor.h
                include/wutils/network/TcpConnection.h
                include/wutils/network/UdpPoint.h
                include/wutils/network/Error.h
                include/wutils/network/ssl/ssl.h
                include/wutils/network/ssl/Error.h

                include/wutils/net/dns/DNS.h
                include/wutils/net/dns/Error.h

                include/wutils/net/http/Method.h
                include/wutils/net/http/Request.h
                include/wutils/net/http/Header.h
                include/wutils/net/http/base/Tools.h
                include/wutils/net/http/Server.h
                include/wutils/net/http/Worker.h
                include/wutils/net/http/Url.h
                include/wutils/net/http/base/RCode.h
                include/wutils/net/http/url/Scheme.h

                include/wutils/timer/Task.h
                include/wutils/timer/EasyContext.h
        )

        add_subdirectory(demo)

        add_subdirectory(test)

        add_subdirectory(tools/googletest-1.13.0)

    elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Unix")
        message("not support Unix!!!")
        return()
    endif ()

endif ()
