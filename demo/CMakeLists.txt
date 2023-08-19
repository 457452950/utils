cmake_minimum_required(VERSION 3.10.0)
project(demo)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(log)
add_subdirectory(wnet)
add_subdirectory(libevent)
add_subdirectory(buffer)
add_subdirectory(json)
add_subdirectory(objectpool)
add_subdirectory(ssl)
add_subdirectory(timer)

