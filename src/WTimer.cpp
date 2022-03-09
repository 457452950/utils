#include "WTimer.hpp"
#include <iostream>
#include <cstring>
#include <errno.h>
#include <exception>

#if defined OS_IS_LINUX

namespace wlb
{

timerfd CreateNewTimerfd()
{
    try
    {
        // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
        return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
        // return ::timerfd_create(CLOCK_REALTIME, 0);
    }
    catch(const std::exception& e)
    {
        // std::cerr << e.what() << '\n';
    }
    return 0;
}

bool SetTimerTime(timerfd fd, 
                    SetTimeFlag flag, 
                    const struct itimerspec* next_time, 
                    struct itimerspec* prev_time)
{
    try
    {
        if ( ::timerfd_settime(fd, (int)flag, next_time, prev_time) == 0)
        {
            return true;
        }
    }
    catch(const std::exception& e)
    {
        // std::cerr << e.what() << '\n';
    }
    return false;
}

////////////////////////////////////////////////////////////////
// timer 

bool WTimer::Start(long time_value, long interval)
{
    if (this->_active)
    {
        return false;
    }

    if (this->_fd == -1)
    {
        this->_fd = CreateNewTimerfd();
        if (this->_fd == -1)
        {
            return false;
        }
    }
    
    struct itimerspec next_time{0};
    next_time.it_value.tv_sec = time_value / 1000L;
    next_time.it_value.tv_nsec = (time_value % 1000L) * 1000'000L;
    next_time.it_interval.tv_sec = interval / 1000L;
    next_time.it_interval.tv_nsec = (interval % 1000L) * 1000'000L;
    if (!SetTimerTime(this->_fd, SetTimeFlag::REL, &next_time))
    {
        
        return false;
    }
    
    if (this->_handlerData == nullptr)
    {
        this->_handlerData = new(std::nothrow) WTimerHandlerData(this, this->_listener, this->_fd);
        if (this->_handlerData == nullptr)
        {
            return false;
        }
    }
    

    this->_handler->AddTimer(this->_handlerData);

    this->_active = true;

    return true;
}
void WTimer::Stop()
{
    if (!this->_active)
        return;
        
    this->_handler->RemoveTimer(this->_handlerData);
    this->_active = false;
}


}   // namespace wlb
#endif
