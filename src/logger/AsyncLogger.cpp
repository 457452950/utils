//
// Created by wlb on 2021/9/17.
//

#include "../../include/AsyncLogger.h"
#include <sys/types.h>
#include <sys/time.h>

//检查文件(所有类型,包括目录和文件)是否存在
//返回1:存在 0:不存在
bool IsFileExist(const char* path)
{
    return !access(path, 0);
}

namespace wlb
{

namespace Log
{

    Logger* Logger::s_Instance = nullptr;
    bool Logger::s_bIsActive = false;
    LOG_LEVEL Logger::s_LogLevel = LOG_LEVEL::L_ERROR;
    char* Logger::s_strFileName = nullptr;

    Logger* Logger::getInstance()
    {
        return s_Instance;
    }

    Logger::Logger()
    {
        initFilePath();
    }

    Logger::~Logger()
    {
        if (m_oStream.is_open())
            m_oStream.close();
    }

    void Logger::initFilePath()
    {
        if (m_oStream.is_open())
        {
            m_oStream.close();
        }

        // get data and time 
        time_t _t = time(NULL);
        auto _time = localtime(&_t);

        char name[256];
        snprintf(name, 256,
                 "log/%s-%d-%02d-%02d-%02d-%02d.%d.log",
                 Logger::s_strFileName,
                 _time->tm_mon+1,
                 _time->tm_mday,
                 _time->tm_hour,
                 _time->tm_min,
                 _time->tm_sec,
                 getpid());

        if (!IsFileExist("log"))
        {
#ifdef WIN32
            mkdir("log");
#else
            ::mkdir("log", 477);
#endif // WIN32

        }
        m_oStream.open(name, std::ios::out);
    }

    LogHelper_ptr Logger::Write(const char* level,
                                const char* file,
                                int lineNo,
                                const char* date,
                                const char*,
                                const char* _func)
    {
        m_mMutex.lock();

        char _dataVal[128];
        char head[256];

#ifdef WIN32
        SYSTEMTIME curTime;
        GetLocalTime(&curTime);
        snprintf(_dataVal, 128, " %d-%d-%d %02d:%02d:%02d.%03ld",
            curTime.wYear + 1900,
            curTime.wMonth + 1,
            curTime.wDay,
            curTime.wHour,
            curTime.wMinute,
            curTime.wSecond,
            curTime.wMilliseconds);
#else

        // get ms
        timeval curTime;
        gettimeofday(&curTime, NULL);

        // get data and time 
        time_t _t = time(NULL);
        auto _time = localtime(&_t);

        snprintf(_dataVal, 128, " %d-%d-%d %02d:%02d:%02d.%03ld %03ld ",
                _time->tm_year + 1900,
                _time->tm_mon + 1,
                _time->tm_mday,
                _time->tm_hour,
                _time->tm_min,
                _time->tm_sec,
                curTime.tv_usec / 1000,
                curTime.tv_usec % 1000);
#endif

        // get head
        snprintf(head, 256,
                 "\n++ %s %s :: %d \n|| %s: [%s] tid=",
                 _dataVal,
                 file,
                 lineNo,
                 level,
                 _func);

        m_sstream << head;
        m_sstream << std::this_thread::get_id() << " :\n  ";
        return std::make_shared<LogHelper>(this);
    }

    void Logger::commit()
    {
        m_sstream << "\n\n";

        m_LogList.push_back(m_sstream.str());
        m_sstream.str("");
        m_sstream.clear();

        m_mMutex.unlock();
        m_condition.notify_all();
    }

    void Logger::Loop()
    {
        while (m_bIsRunning || !m_LogList.empty())
        {
           std::unique_lock<std::mutex> ulock(m_mMutex);
            while (m_LogList.empty())
            {
                m_condition.wait(ulock);
                // 
                if (!m_bIsRunning && m_LogList.empty())
                    break;
            }
            std::string str = m_LogList.front();
            m_LogList.pop_front();

            m_oStream << str;
            m_oStream.flush();

            if ((++m_iTimes %= m_iCheckTimes) == 0)
            {
                if (getFileSize() >= m_maxFileSize)
                    initFilePath();
            }

        }
        
    } 


} // namespace Log


}
