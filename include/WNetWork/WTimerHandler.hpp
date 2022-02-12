#pragma once

#include "../WOS.h"
#include <stdint.h>

namespace wlb{

using timerfd = int32_t;

class WTimerHandler
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        virtual void OnTime(timerfd id) = 0;
    };

    virtual bool Init() = 0;
    virtual void Close()  = 0;

    virtual void AddTimer(Listener* listener, timerfd timer) = 0;
    virtual void RemoveTimer(timerfd timer) = 0;
    virtual void GetAndEmitTimer() = 0;
};

} // namespace wlb
