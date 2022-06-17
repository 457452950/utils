cmake_minimum_required(VERSION 3.10.0)
project(libevent_demo)

include_directories(
    "../../include"
    "../../include/libevent"
)
link_directories("../../lib")

set(CMAKE_CXX_STANDARD 17)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -fsanitize=address")

add_executable(
    libevent_demo
    main.cpp
)

target_link_libraries(libevent_demo event)

