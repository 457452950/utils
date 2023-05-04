//
// Created by wlb on 2021/9/17.
//

#ifndef UTILS_ASYNCLOGGER_H
#define UTILS_ASYNCLOGGER_H

#include <condition_variable>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <thread>
#include <unistd.h> // access()
#include <unordered_set>
#include <vector>

#include "LoggerBase.h"
#include "WSystem.h"


// namespace LOG_TYPE {
// const int8_t L_STDOUT = 1 << 0;
// const int8_t L_FILE   = 1 << 1;
// }

// #define ERROR "ERROR"
// #define WARN "WARN"
// #define INFO  "INFO"
// #define DEBUG "DEBUG"


namespace wlb::Log {

class LogHelper;
using LogHelper_ptr = std::shared_ptr<LogHelper>;

// 生产者-消费者模型
class Logger {
private:
    // Singleton
    Logger();

public:
    ~Logger();
    // no copyable
    void operator=(const Logger &) = delete;
    Logger(const Logger &)         = delete;

    using StringSet  = std::unordered_set<std::string>;
    using StringList = std::list<std::string>;

    // 初始化
    // all tags
    static void Init(int8_t type, LOG_LEVEL level, const char *fileName);
    // use tag registed
    static void Init(int8_t type, LOG_LEVEL level, const char *fileName, StringSet tags);

    static void Stop();
    static void Wait2Exit();

    static Logger   *getInstance();
    static LOG_LEVEL GetLogLevel();

    static bool UseTag();
    static bool ContainTag(const std::string &tag);

    void Loop();

private:
    // log config
    static Logger *instance_;

    LOG_LEVEL log_level_{L_ERROR};
    int8_t    log_type_{LOG_TYPE::L_STDOUT};

    bool      use_tags_{false};
    StringSet tags_;

    // file config
    char         *base_file_name_{nullptr};
    const int64_t max_file_size_{100 * 1024 * 1024}; // 10MB
    const int8_t  max_check_times{10};
    int8_t        check_times_{0}; // 一定轮次检查文件大小
    std::ofstream file_stream_;

    // async
    bool running_{false};

    std::thread            *thread_{nullptr};
    std::mutex              mutex_;
    std::condition_variable con_variable_;

    // Logger helper
public:
    LogHelper_ptr      Write(const char *level, const char *tag, const char *file, int lineNo, const char *_func);
    std::stringstream &GetStream() { return string_stream_; }
    void               commit();

private:
    void    initFilePath();
    int64_t getFileSize() { return file_stream_.tellp(); }

private:
    StringList        log_string_list_;
    std::stringstream string_stream_;
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
    if(Logger::getInstance() && Logger::GetLogLevel() <= (level))                                                      \
        if(!Logger::getInstance()->UseTag() || (Logger::getInstance()->UseTag() && Logger::ContainTag(tag)))           \
    Logger::getInstance()->Write(#level, tag, __FILENAME__, __LINE__, __FUNCTION__)->Get()

} // namespace wlb::Log

/*WARNING !!!
 * Cant use by this:
 *
 *    LOG(xxx) << func();
 *
 *    func() { LOG(xxx); }
 */
#endif // UTILS_ASYNCLOGGER_H
