#pragma once
#ifndef UTILS_ASYNC_LOGGER_H
#define UTILS_ASYNC_LOGGER_H

#include <condition_variable>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>
#include <unistd.h> // access()
#include <unordered_set>
#include <vector>

#include "LoggerBase.h"
#include "wutils/SharedPtr.h"
#include "wutils/System.h"


namespace wutils::Log {

class LogHelper;
using Helper = shared_ptr<LogHelper>;

// 生产者-消费者模型
class Logger {
    // Singleton
public:
    ~Logger();
    // no copyable
    void operator=(const Logger &) = delete;
    Logger(const Logger &)         = delete;

    static Logger *getInstance();

private:
    Logger();
    static Logger *instance_;

    using StringSet  = std::unordered_set<std::string>;
    using StringList = std::list<std::string>;

    // 初始化
public:
    // all tags
    static void Init(uint8_t type, LOG_LEVEL level, const std::string &fileName);
    // use tags
    static void Init(uint8_t type, LOG_LEVEL level, const std::string &fileName, StringSet tags);

public:
    static void Stop();
    static void Wait2Exit();

public:
    LOG_LEVEL GetLogLevel() const;

    bool UseTag() const;
    bool ContainTag(const std::string &tag) const;

private:
    // log config
    LOG_LEVEL log_level_{LERROR};
    uint8_t   log_type_{LOG_TYPE::L_STDOUT};

    bool      use_tags_{false};
    StringSet tags_;


private:
    void    initFilePath();
    int64_t getFileSize() { return file_stream_.tellp(); }

private:
    // file config
    std::string   base_file_name_;
    const int64_t max_file_size_{100 * 1024 * 1024}; // 10MB
    const int8_t  max_check_times{10};
    int8_t        check_times_{0};                   // 一定轮次检查文件大小
    std::ofstream file_stream_;

    StringList        log_string_list_;
    std::stringstream string_stream_;

    // async
private:
    void Loop();

    bool running_{false};

    std::thread                *thread_{nullptr};
    std::recursive_mutex        mutex_;
    std::condition_variable_any con_variable_;

    // Logger helper
public:
    Helper Write(const char *level, const char *tag, const char *file, int lineNo, const char *_func);

private:
    friend class LogHelper;
    std::stringstream &GetStream() { return string_stream_; }
    void               commit();
};

class LogHelper {
public:
    explicit LogHelper(Logger *log) : _log(log) {}
    ~LogHelper() { _log->commit(); }
    std::stringstream &Get() { return _log->GetStream(); };

private:
    Logger *_log;
};

#define LOG(level, tag)                                                                                                \
    if(Logger::getInstance() && Logger::getInstance()->GetLogLevel() <= (level))                                       \
        if(Logger::getInstance()->ContainTag(tag))                                                                     \
    Logger::getInstance()->Write(#level, tag, FILENAME_, __LINE__, __FUNCTION__)->Get()

} // namespace wutils::Log

#endif // UTILS_ASYNC_LOGGER_H
