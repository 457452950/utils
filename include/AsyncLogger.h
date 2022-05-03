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
#include <ctime>

#endif


// #define ERROR "ERROR"
// #define WARN "WARN"
// #define INFO  "INFO"
// #define DEBUG "DEBUG"


bool IsFileExist(const char *path);

namespace wlb::Log {

class LogHelper;

using LogHelper_ptr = std::shared_ptr<LogHelper>;

class Logger {
    // Singleton
public:
    static void Init(LOG_LEVEL level, char *fileName) {
        s_strFileName = new char[strlen(fileName) + 1];
        memcpy(s_strFileName, fileName, strlen(fileName));
        s_strFileName[strlen(fileName)] = '\0';
        s_Instance = new Logger();
        s_LogLevel = level;
        s_Instance->m_bIsRunning = true;
        s_Instance->m_pThread    = new(std::nothrow) std::thread(&Logger::Loop, s_Instance);
        Logger::s_bIsActive = true;
    }
    static void Stop() {
        s_Instance->m_bIsRunning = false;
    }
    static void Wait2Exit() {
        if (s_Instance->m_pThread != nullptr && s_Instance->m_pThread->joinable()) {
            s_Instance->m_pThread->join();
        }
    }
    static Logger *getInstance();
    static bool s_bIsActive;
    void operator=(Logger *) = delete;
    Logger(const Logger &) = delete;

    static LOG_LEVEL s_LogLevel;
private:
    Logger();
    ~Logger();
    static Logger *s_Instance;
    static char   *s_strFileName;

    // Logger
public:
    LogHelper_ptr Write(const char *level,
                        const char *file,
                        int lineNo,
                        const char *date,
                        const char *_time,
                        const char *_func);
    std::stringstream &GetStream() { return m_sstream; }
    void commit();

private:
    void initFilePath();
    int getFileSize() { return m_oStream.tellp(); }
    void Loop();

private:
    const int m_maxFileSize = 100 * 1024 * 1024;  // 10MB
    const int m_iCheckTimes = 10;
    int       m_iTimes{0};

    std::ofstream           m_oStream;
    std::mutex              m_mMutex;
    std::condition_variable m_condition;
    std::thread             *m_pThread{nullptr};
    std::stringstream       m_sstream;
    std::list<std::string>
                            m_LogList;
    bool                    m_bIsRunning{false};
};

class LogHelper {
public:
    explicit LogHelper(Logger *log) { _log = log; }
    ~LogHelper() { _log->commit(); }
    std::stringstream &Get() { return _log->GetStream(); };
private:
    Logger *_log;
};

#define LOG(level)                    \
    if (Logger::s_LogLevel <= level && Logger::s_bIsActive)           \
        Logger::getInstance()->Write(#level, __FILE__, __LINE__,     \
                            __DATE__, __TIME__, __FUNCTION__)->Get()

}

/*WARNING !!!
* Cant use by this:
*
*    LOG(xxx) << func();
*
*    func() { LOG(xxx); }
*/
#endif //MYSERVICE_ASYNCLOGGER_H
