// #include <sys/time.h>
#include "LoggerBase.h"

// #include <cassert>
#include <sstream>

namespace wlb {

namespace Log {

void GetLogFileName(const std::string &base_file_name, char *file_name, int max_len) {
    // get data and time
    time_t _t    = ::time(nullptr);
    auto  *_time = ::localtime(&_t);

    ::snprintf(file_name,
               max_len,
               "log/%s-%d-%02d-%02d-%02d-%02d.%d.log",
               base_file_name.c_str(),
               _time->tm_mon + 1,
               _time->tm_mday,
               _time->tm_hour,
               _time->tm_min,
               _time->tm_sec,
               ::getpid());
}

void MakeMessageHead(const char *file_name,
                     const char *tag,
                     int         line_no,
                     const char *log_level,
                     const char *func_name,
                     char       *head,
                     int         max_len) {

    char        data_val[150];
    std::string _file_name(file_name);

    wlb::GetCurrentTimeFormat(data_val, 150);
    // get head
#ifndef NDEBUG
    ::snprintf(head, max_len, "%s[%s|%s][%s:%d:%s]", data_val, log_level, tag, file_name, line_no, func_name);
#else
    ::snprintf(head, max_len, "%s[%s|%s]", data_val, log_level, tag);
#endif
}

} // namespace Log

} // namespace wlb
