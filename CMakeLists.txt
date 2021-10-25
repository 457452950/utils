cmake_minimum_required(VERSION 3.10.2)
project(util)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
set(LIBRARY_OUTPUT_PATH lib)

add_library(logger      src/logger/Logger.cpp)

add_library(jsonreader  src/json_file_reader/JsonFileReader.cpp)
target_link_libraries(jsonreader logger)

add_library(redis       src/redis/Redis.cpp)
target_link_libraries(redis logger hiredis event)
