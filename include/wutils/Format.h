#pragma once
#ifndef UTILS_WSYSTEM_H
#define UTILS_WSYSTEM_H


#include <cstdio>     // snprintf
#include <ctime>      // timeval localtime() time()
#include <sys/time.h> // gettimeofday()

namespace wutils {

/**
 *获取当前格式化时间
 * @param buff
 * @param max_len
 *
 * @example 2023-6-27 23:22:20.405.559
 */
inline void GetCurrentTimeFormat(char *buff, int max_len) {
    // get ms
    timeval curTime{};
    ::gettimeofday(&curTime, nullptr);

    // get data and time
    time_t _t    = ::time(nullptr);
    auto   _time = ::localtime(&_t);

    ::snprintf(buff,
               max_len,
               "[%d-%d-%d %02d:%02d:%02d.%03ld.%03ld]",
               _time->tm_year + 1900,
               _time->tm_mon + 1,
               _time->tm_mday,
               _time->tm_hour,
               _time->tm_min,
               _time->tm_sec,
               curTime.tv_usec / 1000,
               curTime.tv_usec % 1000);
}


} // namespace wutils

#endif // UTILS_WSYSTEM_H
