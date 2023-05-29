cmake_minimum_required(VERSION 3.10.0)
project(demo)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (LINUX)
    add_subdirectory(Log)
    add_subdirectory(wnet)
    add_subdirectory(libevent)
    add_subdirectory(safe)
    add_subdirectory(buffer)
endif ()

if (WINDOWS)
    message("demo for WINDOWS")
    add_subdirectory(buffer)
endif ()


