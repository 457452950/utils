#include "wutils/WSystem.h"
#include <sys/time.h>

namespace wutils {

bool mkdir(const std::string &path) {
    auto res = ::mkdir(path.c_str(), 477);
    if(res == 0) {
        return true;
    }
    return false;
}

bool IsFileExist(const std::string &path) { return !access(path.c_str(), 0); }

void GetCurrentTimeFormat(char *buff, int max_len) {
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
