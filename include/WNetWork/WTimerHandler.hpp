#pragma once

#include "../WOS.h"
#include <stdint.h>


namespace wlb{

using timerfd = int32_t;
class WTimerHandlerData;
class WTimer;

class WTimerHandler
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        virtual void OnTime(WTimer* _timer) = 0;
    };

    virtual bool Init() = 0;
    virtual void Close()  = 0;
    virtual void Destroy() = 0;

    virtual void AddTimer(WTimerHandlerData* data) = 0;
    virtual void RemoveTimer(WTimerHandlerData* data) = 0;
    virtual void GetAndEmitTimer(int32_t timeout = 0) = 0;
};

struct WTimerHandlerData
{
    WTimerHandler::Listener *listener;
    timerfd& _timerfd;
    WTimer* _timer;
    WTimerHandlerData(WTimer* timer, WTimerHandler::Listener *listener, timerfd& timer_fd)
        : _timerfd(timer_fd), listener(listener),_timer(timer)  {}
};


} // namespace wlb
