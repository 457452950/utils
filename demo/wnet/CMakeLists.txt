cmake_minimum_required(VERSION 3.10.0)
project(wnet_demo)

include_directories(
        "../../include"
        "../../include/libevent"
)
link_directories("../../lib")

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -fsanitize=address")

add_executable(
        wnet_demo
        main.cpp

        test_ipv6.h
        test_tcp_aconnection.h
        test_tcp_connection.h
        test_timer.h

        test_echo_tcp.h)

target_link_libraries(wnet_demo wutils)

