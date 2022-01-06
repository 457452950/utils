//
// Created by wlb on 2021/9/17.
//

#include "../../include/Logger.h"


namespace wlb
{

namespace Log
{

    Logger* Logger::s_Instance = nullptr;
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
            mkdir("log", 477);
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

        GetCurrentTime(_dataVal);

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

} // namespace Log

}
