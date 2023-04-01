cmake_minimum_required(VERSION 3.10.0)
project(demo)

if (LINUX)
    add_subdirectory(Log)
    add_subdirectory(wnet)
    add_subdirectory(libevent)
    add_subdirectory(safe)
endif()

if (WINDOWS)
    message("demo for WINDOWS")
endif()


