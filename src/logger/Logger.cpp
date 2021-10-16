//
// Created by wlb on 2021/9/17.
//

#include "../../include/Logger.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//检查文件(所有类型,包括目录和文件)是否存在
//返回1:存在 0:不存在
int IsFileExist(const char* path)
{
    return !access(path, F_OK);
}

namespace wlb
{

    Logger* Logger::s_Instance = new Logger;

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
                 "log/%02d%02d%02d.%d.log",
                 _time->tm_mday,
                 _time->tm_hour,
                 _time->tm_min,
                 getpid());

        if (!IsFileExist("log"))
        {
            mkdir("log", 477);
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

        char head[256];

        // get ms
        timeval curTime;
        gettimeofday(&curTime, NULL);

        // get data and time 
        time_t _t = time(NULL);
        auto _time = localtime(&_t);

        char _dataVal[128];
        snprintf(_dataVal, 128, " %d-%d-%d %02d:%02d:%02d.%03ld %03ld ",
                _time->tm_year + 1900,
                _time->tm_mon + 1,
                _time->tm_mday,
                _time->tm_hour,
                _time->tm_min,
                _time->tm_sec,
                curTime.tv_usec / 1000,
                curTime.tv_usec % 1000);

        // get head
        snprintf(head, 256,
                 "\n++ %s %s :: %d \n|| %s: [%s] tid=",
                 _dataVal,
                 file,
                 lineNo,
                 level,
                 _func);

        m_oStream << head;
        m_oStream << std::this_thread::get_id() << " :\n  ";
        return std::make_shared<LogHelper>(this);
    }

    void Logger::commit()
    {
        m_oStream << "\n\n";

        m_oStream.flush();

        if ((++m_iTimes %= m_iCheckTimes) == 0)
        {
            if (getFileSize() >= m_maxFileSize)
                initFilePath();
        }

        m_mMutex.unlock();
    }

}
