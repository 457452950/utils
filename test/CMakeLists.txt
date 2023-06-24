project(google_unit_tests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../include ../include/libevent)

add_executable(
        unit_test
        wutils_network_tools.cpp
)

# linking Google_Tests_run with DateConverter_lib which will be tested

target_link_libraries(unit_test gtest gtest_main)