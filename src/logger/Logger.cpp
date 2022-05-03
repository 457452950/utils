//
// Created by wlb on 2021/9/17.
//

#include "../../include/Logger.h"

namespace wlb::Log {

Logger    *Logger::s_Instance    = nullptr;
LOG_LEVEL Logger::s_LogLevel     = LOG_LEVEL::L_ERROR;
char      *Logger::s_strFileName = nullptr;
bool      Logger::s_IsActive     = false;

Logger *Logger::getInstance() {
    return s_Instance;
}

Logger::Logger() {
    initFilePath();
}

Logger::~Logger() {
    if (m_oStream.is_open()) {
        m_oStream.close();
    }
}

void Logger::initFilePath() {
    if (m_oStream.is_open()) {
        m_oStream.close();
    }

    char name[256];
    wlb::GetLogFileName(Logger::s_strFileName, name, 256);

    if (!IsFileExist("log")) {
        wlb::mkdir("log");

    }
    m_oStream.open(name, std::ios::out);
}

LogHelper_ptr Logger::Write(const char *level,
                            const char *file,
                            int lineNo,
                            const char *_func) {
    m_mMutex.lock();

    char head[256];

    wlb::MakeMessageHead(file, lineNo, level, _func, head, 256);

    m_oStream << head;
    m_oStream << "[tid:" << std::this_thread::get_id() << "]:";
    return std::make_shared<LogHelper>(this);
}

void Logger::commit() {
    m_oStream << "\n";

    m_oStream.flush();

    if ((++m_iTimes %= m_iCheckTimes) == 0) {
        if (getFileSize() >= m_maxFileSize) {
            initFilePath();
        }
    }

    m_mMutex.unlock();
}

}
