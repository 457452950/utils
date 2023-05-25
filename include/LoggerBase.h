#pragma once
#ifndef UTILS_LOGGER_BASE_H
#define UTILS_LOGGER_BASE_H

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h> // access()

#include "WSystem.h"

#define FILENAME(x) strrchr(x, '/') ? (strrchr(x, '/') + 1) : x
#define __FILENAME__ FILENAME(__FILE__)

namespace wlb {


namespace Log {

namespace LOG_TYPE {
constexpr int8_t L_STDOUT = 1 << 0;
constexpr int8_t L_FILE   = 1 << 1;
} // namespace LOG_TYPE

enum LOG_LEVEL : uint8_t {
    LDEBUG = 1 << 0,
    LINFO  = 1 << 1,
    LWARN  = 1 << 2,
    LERROR = 1 << 3,
    LFATAL = 1 << 4,
};


void GetLogFileName(const std::string &base_file_name, char *file_name, int max_len);
void MakeMessageHead(const char *file_name,
                     const char *tag,
                     int         line_no,
                     const char *log_level,
                     const char *func_name,
                     char       *head,
                     int         max_len);

} // namespace Log
} // namespace wlb

#endif // UTILS_LOGGER_BASE_H
