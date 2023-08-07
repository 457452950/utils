#pragma once
#ifndef UTIL_FORMATLOGGER_H
#define UTIL_FORMATLOGGER_H

#include <cstdarg>
#include <filesystem>

#include <fmt/compile.h>

#include "Logger.h"

namespace fs = std::filesystem;
using namespace wutils::log;


#ifdef LOG_BASIC_PATH
const std::string com_path = fmt::format(FMT_COMPILE("{}"), LOG_BASIC_PATH);
#else
const std::string com_path = fmt::format(FMT_COMPILE("/"));
#endif

void LogWriteHead(std::string       &buffer,
                  const char        *level,
                  const char        *tag,
                  const std::string &file,
                  int                lineNo,
                  const char        *_func) {
    buffer.append(MakeMessageHead(level, tag, file, lineNo, _func));
}

void LogWrite(std::string &buffer, const char *format, ...) {
    char f[4096];

    va_list argp;
    va_start(argp, format);               /* 将可变长参数转换为va_list */
    auto len = vsprintf(f, format, argp); /* 将va_list传递给子函数 */
    va_end(argp);

    buffer.append(f, len);
}

#define LOG_(LEVEL, tag, ...)                                                                                          \
    do {                                                                                                               \
        if(Logger::GetInstance() && Logger::GetInstance()->GetLogLevel() <= (LEVEL))                                   \
            if(Logger::GetInstance()->ContainTag(tag)) {                                                               \
                std::string str;                                                                                       \
                LogWriteHead(str,                                                                                      \
                             #LEVEL,                                                                                   \
                             tag,                                                                                      \
                             fs::path(__FILE__).lexically_relative(com_path).string(),                                 \
                             __LINE__,                                                                                 \
                             __FUNCTION__);                                                                            \
                LogWrite(str, ##__VA_ARGS__);                                                                          \
                Logger::GetInstance()->Commit(str);                                                                    \
            }                                                                                                          \
    } while(0)


#define LOG_INF(tag, ...) LOG_(LINFO, tag, ##__VA_ARGS__)

#define LOG_DBG(tag, ...) LOG_(LDEBUG, tag, ##__VA_ARGS__)

#define LOG_WRN(tag, ...) LOG_(LWARN, tag, ##__VA_ARGS__)

#define LOG_ERR(tag, ...) LOG_(LERROR, tag, ##__VA_ARGS__)

#define LOG_FAL(tag, ...) LOG_(LFATAL, tag, ##__VA_ARGS__)


#endif // UTIL_FORMATLOGGER_H
