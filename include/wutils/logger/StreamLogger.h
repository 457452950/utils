#pragma once
#ifndef UTIL_STREAMLOGGER_H
#define UTIL_STREAMLOGGER_H

#include <filesystem>

#include "Logger.h"
#include "wutils/SharedPtr.h"

namespace fs = std::filesystem;
using namespace wutils::log;

#ifdef LOG_BASIC_PATH
const std::string com_path = LOG_BASIC_PATH;
#else
#ifndef NDEBUG
#pragma message("do not defined LOG_BASIC_PATH! will use '/' for basic path")
#endif
const std::string com_path = "/";
#endif

class LogHelper {
public:
    explicit LogHelper() = default;
    ~LogHelper() { Logger::GetInstance()->Commit(o.str()); }
    std::stringstream &
    Write(const char *level, const char *tag, const std::string &file, int lineNo, const char *_func) {
        char head[256]{0};
        MakeMessageHead(file.data(), tag, lineNo, level, _func, head, 256);

        o << head;
        o << "[tid:" << std::this_thread::get_id() << "]:";
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
