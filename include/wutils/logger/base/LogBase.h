#pragma once
#ifndef UTILS_LOGGER_BASE_H
#define UTILS_LOGGER_BASE_H

#include <string>
#include <unistd.h> // getpid
#include <ctime>

#include <fmt/printf.h>
#include <fmt/chrono.h>
#include <fmt/ostream.h>

#include "wutils/Format.h"
#include "wutils/SharedPtr.h"

namespace wutils::log {

namespace LOG_TYPE {
constexpr uint8_t L_STDOUT = 1 << 0;
constexpr uint8_t L_FILE   = 1 << 1;
} // namespace LOG_TYPE

enum LOG_LEVEL : uint8_t {
    LDEBUG = 1 << 0,
    LINFO  = 1 << 1,
    LWARN  = 1 << 2,
    LERROR = 1 << 3,
    LFATAL = 1 << 4,
};

inline std::string GetLogFileName(const std::string &base_file_name) {
    // get data and time
    std::time_t t = std::time(nullptr);
    std::tm     now_tm;
    localtime_r(&t, &now_tm);

    return fmt::format("{}-{:%m-%d-%H-%M-%S}-p{}", base_file_name, now_tm, ::getpid());
}

inline std::string
MakeMessageHead(const char *level, const char *tag, const std::string &file_name, int line_no, const char *func_name) {
    timeval curTime{};
    ::gettimeofday(&curTime, nullptr);

    std::time_t t = std::time(nullptr);
    std::tm     now_tm;
    localtime_r(&t, &now_tm);

#ifndef NDEBUG
    // doc:https://fmt.dev/latest/syntax.html#chrono-specs
    // [2023-08-05 21:58:30.242.077][LINFO|main][main.cpp:57:operator()][tid:139909122745920]
    return fmt::format("[{:%Y-%m-%d %T}.{:03}.{:03}][{}|{}][{}:{}:{}][tid:{}]",
                       now_tm,
                       curTime.tv_usec / 1000,
                       curTime.tv_usec % 1000,
                       level,
                       tag,
                       file_name,
                       line_no,
                       func_name,
                       fmt::streamed(std::this_thread::get_id()));
#else
    // [2023-08-05 22:05:48.852.603][LINFO|main][tid:139909122745920]
    return fmt::format("[{:%Y-%m-%d %T}.{:03}.{:03}][{}|{}][tid:{}]",
                       now_tm,
                       curTime.tv_usec / 1000,
                       curTime.tv_usec % 1000,
                       level,
                       tag,
                       fmt::streamed(std::this_thread::get_id()));
#endif
}

} // namespace wutils::log

#endif // UTILS_LOGGER_BASE_H
