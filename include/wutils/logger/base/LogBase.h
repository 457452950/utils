#pragma once
#ifndef UTILS_LOGGER_BASE_H
#define UTILS_LOGGER_BASE_H

#include <string>

#include "wutils/Format.h"
#include "wutils/SharedPtr.h"

// #define FILENAME(x) strrchr(x, '/') ? (strrchr(x, '/') + 1) : x
// #define FILENAME_ FILENAME(__FILE__)

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


inline void GetLogFileName(const std::string &base_file_name, char *file_name, int max_len) {
    // get data and time
    time_t _t    = ::time(nullptr);
    auto  *_time = ::localtime(&_t);

    ::snprintf(file_name,
               max_len,
               "%s-%d-%02d-%02d-%02d-%02d:%d",
               base_file_name.c_str(),
               _time->tm_mon + 1,
               _time->tm_mday,
               _time->tm_hour,
               _time->tm_min,
               _time->tm_sec,
               ::getpid());
}

inline void GetLogFileName(const std::string &base_file_name, std::string &file_name) {
    // get data and time
    time_t _t    = ::time(nullptr);
    auto  *_time = ::localtime(&_t);

    unique_ptr<char[]> buf = make_unique<char[]>(256);

    auto len = ::snprintf(buf.get(),
                          256,
                          "%s-%d-%02d-%02d-%02d-%02d:%d",
                          base_file_name.c_str(),
                          _time->tm_mon + 1,
                          _time->tm_mday,
                          _time->tm_hour,
                          _time->tm_min,
                          _time->tm_sec,
                          ::getpid());

    file_name.assign(buf.get(), len);
}
inline void MakeMessageHead(const char *file_name,
                            const char *tag,
                            int         line_no,
                            const char *log_level,
                            const char *func_name,
                            char       *head,
                            int         max_len) {

    char        data_val[150];
    std::string _file_name(file_name);

    wutils::GetCurrentTimeFormat(data_val, 150);
    // get head
#ifndef NDEBUG
    ::snprintf(head, max_len, "%s[%s|%s][%s:%d:%s]", data_val, log_level, tag, file_name, line_no, func_name);
#else
    ::snprintf(head, max_len, "%s[%s|%s]", data_val, log_level, tag);
#endif
}

} // namespace wutils::log

#endif // UTILS_LOGGER_BASE_H
