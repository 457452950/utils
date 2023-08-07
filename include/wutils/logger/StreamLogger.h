#pragma once
#ifndef UTIL_STREAMLOGGER_H
#define UTIL_STREAMLOGGER_H

#include <filesystem>

#include <fmt/compile.h>

#include "Logger.h"
#include "wutils/SharedPtr.h"

namespace fs = std::filesystem;
using namespace wutils::log;

#ifdef LOG_BASIC_PATH
const std::string com_path = fmt::format(FMT_COMPILE("{}"), LOG_BASIC_PATH);
#else
const std::string com_path = fmt::format(FMT_COMPILE("/"));
#endif

class LogHelper {
public:
    explicit LogHelper() = default;
    ~LogHelper() { Logger::GetInstance()->Commit(o.str()); }

    std::stringstream &
    Write(const char *level, const char *tag, const std::string &file, int lineNo, const char *_func) {
        o << MakeMessageHead(level, tag, file, lineNo, _func);
        return o;
    }

private:
    std::stringstream o;
};

#define LOG(level, tag)                                                                                                \
    if(Logger::GetInstance() && Logger::GetInstance()->GetLogLevel() <= (level))                                       \
        if(Logger::GetInstance()->ContainTag(tag))                                                                     \
    wutils::make_unique<LogHelper>()->Write(                                                                           \
            #level, tag, fs::path(__FILE__).lexically_relative(com_path).string(), __LINE__, __FUNCTION__)


#endif // UTIL_STREAMLOGGER_H
