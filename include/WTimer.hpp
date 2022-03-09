#pragma once
#include "WOS.h"

#if defined(OS_IS_LINUX)

#include "unistd.h"
#include <sys/timerfd.h>
#include "WNetWork/WTimerHandler.hpp"


namespace wlb
{

// using timerfd = int;
// struct itimerspec
// {
//   struct timespec it_interval;
//   struct timespec it_value;
// };

timerfd CreateNewTimerfd();

enum class SetTimeFlag
{
    REL = 0,    // 相对时间
    ABS = 1,    // 绝对时间
};
bool SetTimerTime(timerfd fd, SetTimeFlag flag, const struct itimerspec* next_time, struct itimerspec* prev_time = nullptr);





////////////////////////////////////////////////////////////////

// class WTimer  使用精度ms

struct WTimerHandlerData;
class WTimerHandler;

class WTimer
{
public:
    explicit WTimer(WTimerHandler::Listener* listener, WTimerHandler* handler) 
        : _listener(listener), _handler(handler) {};
    WTimer(const WTimer& other) = delete;
    WTimer& operator=(const WTimer& other) = delete;
    virtual ~WTimer() {
        if (this->_fd != -1) ::close(this->_fd);
        if (this->_handlerData != nullptr) delete this->_handlerData;
    };

    bool Start(long time_value, long interval = 0);
    void Stop();

    inline bool IsActive() { return this->_active; }
protected:
    timerfd _fd{-1};
    bool _active{false};
    WTimerHandler* _handler{nullptr};
    WTimerHandler::Listener* _listener{nullptr};

    WTimerHandlerData* _handlerData{nullptr};
};

} // namespace wlb

#endif