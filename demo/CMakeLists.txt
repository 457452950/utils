cmake_minimum_required(VERSION 3.10.0)
project(demo)

if (LINUX)
    add_subdirectory(Log)
    add_subdirectory(wnet)
    add_subdirectory(libevent)
endif()

if (WINDOWS)
    message("demo for WINDOWS")
    add_subdirectory(safe)
endif()


