// #include <sys/time.h>
#include "LoggerBase.h"
#include <sstream>

namespace wlb {

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
void MakeMessageHead(
        const char *file_name, int line_no, const char *log_level, const char *func_name, char *head, int max_len) {

    char        data_val[128];
    std::string _file_name(file_name);

    wlb::GetCurrentTimeFormat(data_val, 128);

    // get head
    ::snprintf(head, max_len, "%s[%s][%s:%d:%s] ", data_val, log_level, file_name, line_no, func_name);
}

namespace Log {}

} // namespace wlb
