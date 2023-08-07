cmake_minimum_required(VERSION 3.10.0)
project(log_demo)

include_directories(
        "../../include"
        "../../include/libevent"
)
link_directories("../../lib")

add_compile_definitions(LOG_BASIC_PATH="${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -Wall -fsanitize=address")

add_executable(
        log_demo
        main.cpp
)

target_link_libraries(log_demo fmt)

