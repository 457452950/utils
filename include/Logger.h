//
// Created by wlb on 2021/9/17.
//

#ifndef MYSERVICE_LOGGER_H
#define MYSERVICE_LOGGER_H

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <memory>

#ifdef WIN32

#include <windows.h>    // GetLocalTime
#include <direct.h>     // mkdir
#include <io.h>         // access
#else   // linux

#include <unistd.h>     // access()
#include <sys/time.h>

#endif


// #define ERROR "ERROR"
// #define WARN "WARN"
// #define INFO  "INFO"
// #define DEBUG "DEBUG"


bool IsFileExist(const char* path);

namespace wlb
{

namespace Log
{
    
    enum LOG_LEVEL : uint8_t
    {
        L_DEBUG = 1 << 0,
        L_INFO  = 1 << 1,
        L_WARN  = 1 << 2,
        L_ERROR = 1 << 3,
        L_FATAL = 1 << 4,
    };

    class LogHelper;

    using LogHelper_ptr = std::shared_ptr<LogHelper>;

    class Logger
    {
        // Singleton
    public:
        static void Init(LOG_LEVEL level, char* fileName){
            s_LogLevel = level;
            s_strFileName = fileName;
            s_Instance = new Logger();
        }
        static Logger* getInstance();
        void operator=(Logger*) = delete;
        Logger(const Logger&) = delete;

        static LOG_LEVEL    s_LogLevel;
    private:
        Logger();
        ~Logger();
        static Logger*      s_Instance;
        static char*        s_strFileName;

        // Logger
    public:
        LogHelper_ptr   Write(const char* level,
                              const char* file,
                              int           lineNo,
                              const char* date,
                              const char* _time,
                              const char* _func);
        std::ofstream& GetStream() { return m_oStream; }
        void            commit();

    private:
        void            initFilePath();
        int             getFileSize() { return m_oStream.tellp(); }

    private:
        const int       m_maxFileSize = 100 * 1024 * 1024;  // 10MB
        const int       m_iCheckTimes = 10;
        int             m_iTimes{ 0 };

        
        std::ofstream   m_oStream;
        std::mutex      m_mMutex;
    };

    class LogHelper
    {
    public:
        LogHelper(Logger* log) { _log = log; }
        ~LogHelper() { _log->commit(); }
        std::ofstream& Get() { return _log->GetStream(); };
    private:
        Logger* _log;
    };

#define LOG(level)                    \
    if (Logger::s_LogLevel <= level)           \
        Logger::getInstance()->Write(#level, __FILE__, __LINE__,     \
                            __DATE__, __TIME__, __FUNCTION__)->Get()

                        
        
} // namespace Log

}

/*WARNING !!!
* Cant use by this:
*
*    LOG(xxx) << func();
*
*    func() { LOG(xxx); }
*/
#endif //MYSERVICE_LOGGER_H
