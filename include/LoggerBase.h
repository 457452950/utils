#pragma once

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>

#ifdef WIN32

#include <windows.h>    // GetLocalTime
#include <direct.h>     // mkdir
#include <io.h>         // access
#else   // linux

#include <unistd.h>     // access()
#include <sys/time.h>

#endif

namespace wlb
{

bool IsFileExist(const char* path);
void GetCurrentTime(char buff[128]);

namespace Log
{

    enum LOG_LEVEL : uint8_t
    {
        L_DEBUG = 1 << 0,
        L_INFO  = 1 << 1,
        L_WARN  = 1 << 2,
        L_ERROR = 1 << 3,
        L_FATAL = 1 << 4,
    };






}   // namespace Log


}   // namespace wlb

