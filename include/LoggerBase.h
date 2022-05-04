#pragma once

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <string>

#include "WSystem.h"

#ifdef OS_IS_WINDOWS

#include <windows.h>    // GetLocalTime
#include <direct.h>     // mkdir
#include <io.h>         // access

  #define FILENAME(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x
#else   // linux

#include <unistd.h>     // access()
#include <ctime>

  #define FILENAME(x) strrchr(x,'/')?(strrchr(x,'/')+1):x
#endif

#define __FILENAME__ FILENAME(__FILE__)

namespace wlb {

void GetLogFileName(const std::string &base_file_name, char *file_name, int max_len);
void MakeMessageHead(const char *file_name,
                     int line_no,
                     const char *log_level,
                     const char *func_name,
                     char *head,
                     int max_len);

namespace Log {

namespace LOG_TYPE {
    const int8_t L_STDOUT = 1 << 0;
    const int8_t L_FILE   = 1 << 1;
}

enum LOG_LEVEL : uint8_t {
    L_DEBUG = 1 << 0,
    L_INFO  = 1 << 1,
    L_WARN  = 1 << 2,
    L_ERROR = 1 << 3,
    L_FATAL = 1 << 4,
};

}   // namespace Log


}   // namespace wlb

