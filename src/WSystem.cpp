#include "WSystem.h"
#include <sys/time.h>

namespace wlb {

bool mkdir(const std::string &path) {
#ifdef OS_IS_WINDOWS
    auto res = ::_mkdir(path.c_str());
#elif OS_IS_LINUX
    auto res = ::mkdir(path.c_str(), 477);
#endif
    if(res == 0) {
        return true;
    }
    return false;
}

bool IsFileExist(const std::string &path) { return !access(path.c_str(), 0); }

void GetCurrentTime(char *buff, int max_len) {

#ifdef OS_IS_WINDOWS
    SYSTEMTIME curTime;
    ::GetLocalTime(&curTime);
    ::snprintf(buff,
               max_len,
               "[%d-%d-%d %02d:%02d:%02d-%03ld]",
               curTime.wYear + 1900,
               curTime.wMonth + 1,
               curTime.wDay,
               curTime.wHour,
               curTime.wMinute,
               curTime.wSecond,
               curTime.wMilliseconds);
#else
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
#endif
}

} // namespace wlb
