cmake_minimum_required(VERSION 3.10.0)
project(buffer_demo)

include_directories(
        "../../include"
        "../../include/libevent"
)
link_directories("../../lib")

set(CMAKE_CXX_STANDARD 17)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -fsanitize=address")

add_executable(
        buffer_demo
        main.cpp
)

target_link_libraries(buffer_demo wutils)

