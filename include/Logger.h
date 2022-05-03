//
// Created by wlb on 2021/9/17.
//

#ifndef MYSERVICE_LOGGER_H
#define MYSERVICE_LOGGER_H

#include "LoggerBase.h"

// #define ERROR "ERROR"
// #define WARN "WARN"
// #define INFO  "INFO"
// #define DEBUG "DEBUG"



namespace wlb::Log {

// enum LOG_LEVEL : uint8_t
// {
//     L_DEBUG = 1 << 0,
//     L_INFO  = 1 << 1,
//     L_WARN  = 1 << 2,
//     L_ERROR = 1 << 3,
//     L_FATAL = 1 << 4,
// };

class LogHelper;

using LogHelper_ptr = std::shared_ptr<LogHelper>;

class Logger {
    // Singleton
public:
    static void Init(LOG_LEVEL level, char *fileName) {
        printf("sync Logger");
        s_LogLevel    = level;
        s_strFileName = fileName;
        s_Instance    = new Logger();
        s_IsActive    = true;
    }
    static Logger *getInstance();
    void operator=(Logger *) = delete;
    Logger(const Logger &) = delete;

    static LOG_LEVEL s_LogLevel;
    static bool      s_IsActive;
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
                        const char *_func);
    std::ofstream &GetStream() { return m_oStream; }
    void commit();

private:
    void initFilePath();
    int getFileSize() { return m_oStream.tellp(); }

private:
    const int m_maxFileSize = 100 * 1024 * 1024;  // 10MB
    const int m_iCheckTimes = 10;
    int       m_iTimes{0};

    std::ofstream m_oStream;
    std::mutex    m_mMutex;
};

class LogHelper {
public:
    explicit LogHelper(Logger *log) : _log(log) {}
    ~LogHelper() { _log->commit(); }
    std::ofstream &Get() { return _log->GetStream(); };
private:
    Logger *_log;
};

#define LOG(level)                    \
    if (Logger::s_LogLevel <= level && Logger::s_IsActive) \
        Logger::getInstance()->Write(#level, __FILE__, __LINE__, \
                             __FUNCTION__)->Get()

}

/*WARNING !!!
* Cant use by this:
*
*    LOG(xxx) << func();
*
*    func() { LOG(xxx); }
*/
#endif //MYSERVICE_LOGGER_H
