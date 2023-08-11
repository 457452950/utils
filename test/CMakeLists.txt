cmake_minimum_required(VERSION 3.20)
project(Test)

set(CMAKE_CXX_STANDARD 17)

include_directories(../include ../tools/googletest-1.13.0/googletest/include ../tools/googletest-1.13.0/googlemock/include)
link_directories("../lib")

add_executable(google_test_run test_net_dns.cpp)

target_link_libraries(google_test_run gtest gtest_main)


