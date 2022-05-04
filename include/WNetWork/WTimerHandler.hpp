#pragma once

#include "../WOS.h"
#include <cstdint>

namespace wlb {

using timerfd = int32_t;
struct WTimerHandlerData;
class WTimer;

class WTimerHandler {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void OnTime(WTimer *_timer) = 0;
    };

    virtual ~WTimerHandler() = default;
    virtual bool Init() = 0;
    virtual void Close() = 0;

    virtual void AddTimer(WTimerHandlerData *data) = 0;
    virtual void RemoveTimer(WTimerHandlerData *data) = 0;
    virtual void GetAndEmitTimer(int32_t timeout) = 0;
};

struct WTimerHandlerData {
    WTimerHandler::Listener *listener;
    timerfd                 &timer_fd_;
    WTimer                  *timer_;
    WTimerHandlerData(WTimer *timer, WTimerHandler::Listener *listener, timerfd &timer_fd)
            : listener(listener), timer_fd_(timer_fd), timer_(timer) {}
};

} // namespace wlb
