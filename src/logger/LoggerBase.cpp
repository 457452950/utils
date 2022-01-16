#include "LoggerBase.h"

namespace wlb
{


//检查文件(所有类型,包括目录和文件)是否存在
//返回1:存在 0:不存在
bool IsFileExist(const char* path)
{
    return !access(path, 0);
}

void GetCurrentTime(char buff[128])
{

#ifdef WIN32
        SYSTEMTIME curTime;
        GetLocalTime(&curTime);
        snprintf(buff, 128, " %d-%d-%d %02d:%02d:%02d.%03ld",
            curTime.wYear + 1900,
            curTime.wMonth + 1,
            curTime.wDay,
            curTime.wHour,
            curTime.wMinute,
            curTime.wSecond,
            curTime.wMilliseconds);
#else
        // get ms
        timeval curTime;
        gettimeofday(&curTime, NULL);

        // get data and time 
        time_t _t = time(NULL);
        auto _time = localtime(&_t);

        snprintf(buff, 128, " %d-%d-%d %02d:%02d:%02d.%03ld %03ld ",
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



namespace Log
{












}

}



