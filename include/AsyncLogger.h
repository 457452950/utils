//
// Created by wlb on 2021/9/17.
//

#ifndef MYSERVICE_ASYNCLOGGER_H
#define MYSERVICE_ASYNCLOGGER_H

#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <list>
#include <cstring>
#include "LoggerBase.h"

#ifdef WIN32
  #include <windows.h>    // GetLocalTime
  #include <direct.h>     // mkdir
  #include <io.h>         // access
#else   // linux
  #include <unistd.h>     // access()
#endif


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
    void operator=(Logger *) = delete;
    Logger(const Logger &) = delete;

    // 初始化
    static void Init(int8_t type, LOG_LEVEL level, char *fileName);
    static void Stop();
    static void Wait2Exit();

    static Logger *getInstance();
    static bool IsActive();
    static LOG_LEVEL GetLogLevel();
    void Loop();
private:
    static Logger           *instance_;
    bool                    active_{false};
    LOG_LEVEL               log_level_{L_ERROR};
    int8_t                  log_type_{LOG_TYPE::L_STDOUT};
    // file log
    char                    *base_file_name_{nullptr};
    // Check file size
    const int64_t           max_file_size_  = 100 * 1024 * 1024;  // 10MB
    const int8_t            max_check_times = 10;
    int8_t                  check_times_{0};
    // async
    std::ofstream           file_stream_;
    bool                    running_{false};
    std::thread             *thread_{nullptr};
    std::mutex              mutex_;
    std::condition_variable con_variable_;

    // Logger
public:
    LogHelper_ptr Write(const char *level,
                        const char *file,
                        int lineNo,
                        const char *_func);
    std::stringstream &GetStream() { return string_stream_; }
    void commit();

private:
    void initFilePath();
    int64_t getFileSize() { return file_stream_.tellp(); }

private:
    std::list<std::string> log_string_list_;
    std::stringstream      string_stream_;
};

class LogHelper {
public:
    explicit LogHelper(Logger *log) : _log(log) {}
    ~LogHelper() { _log->commit(); }
    std::stringstream &Get() { return _log->GetStream(); };
private:
    Logger *_log;
};

#define LOG(level)                    \
    if (Logger::GetLogLevel() <= (level) && Logger::IsActive())           \
        Logger::getInstance()->Write(#level, __FILENAME__, __LINE__,     \
                            __FUNCTION__)->Get()

}

/*WARNING !!!
* Cant use by this:
*
*    LOG(xxx) << func();
*
*    func() { LOG(xxx); }
*/
#endif //MYSERVICE_ASYNCLOGGER_H
