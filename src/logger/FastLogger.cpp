#include "FastLogger.h"

namespace wlb
{
namespace Log
{

FastLogger* FastLogger::_instance = nullptr;

bool FastLogger::Init(LOG_LEVEL level, const char* file_name)
{
    if (FastLogger::_instance != nullptr)
    {
        return false;
    }

    FastLogger::_instance = new(std::nothrow) FastLogger();
    if (_instance == nullptr)
    {
        std::cout << "Log new failed" << std::endl;
        return false;
    }

    _instance->_file_name = std::string(file_name);
    _instance->_log_level = level;

    _instance->initFilePath();

    _instance->_dataFlag.store(0);
    _instance->_running = true;
    _instance->_pThrdFlusher = new(std::nothrow) std::thread(&FastLogger::Loop, _instance);

    return true;
}

void FastLogger::initFilePath()
{

    if (this->_ofstream.is_open())
    {
        this->_ofstream.close();
    }

    // get data and time 
    time_t _t = time(NULL);
    auto _time = localtime(&_t);

    char name[256];
    snprintf(name, 256,
            "log/%s-%d-%02d-%02d-%02d-%02d.%d.log",
            this->_file_name.c_str(),
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

    this->_ofstream.open(name, std::ios::out);
}

void FastLogger::Loop()
{
    while ( this->_running || ! this->_dataFlag.load() == 0)
    {
        std::unique_lock<std::mutex> ulock(_mutex);
        _condition.wait(ulock);

        if ( !_running && this->_dataFlag.load() == 0)
            break;
            
        _ofstream.flush();

        if ((++_times %= _checkTimes) == 0)
        {
            if (getFileSize() >= _maxFileSize)
                initFilePath();
        }
    }
}

std::ofstream& FastLogger::Write()
{
    // sprintf
}

}


} // namespace wlb



