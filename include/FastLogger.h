#pragma once

#include "LoggerBase.h"

namespace wlb
{

namespace Log
{

class FastLogger
{
    // 单例模式
private:
    FastLogger() = default;
    static FastLogger* _instance;
public:
    ~FastLogger();
    static bool Init(LOG_LEVEL level, const char* file_name);
    static FastLogger* GetInstance();


public:
    void initFilePath();
    std::ofstream& Write();
private:
    void Loop();
    int  getFileSize() { return _ofstream.tellp(); }
private:
    std::string _file_name;
    LOG_LEVEL _log_level;

    std::ofstream _ofstream;
    std::thread* _pThrdFlusher;
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _running{false};
    std::atomic<uint16_t> _dataFlag;

    std::uint16_t _times{0};
    std::uint16_t _checkTimes{10};
    std::uint32_t _maxFileSize{100'000'000L};
};


}
}

