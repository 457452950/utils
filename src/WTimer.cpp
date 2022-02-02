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

bool WTimer::operator==(const timerfd& rhs)
{
    return (this->_fd == rhs);
}
bool WTimer::operator!=(const timerfd& rhs)
{
    return (this->_fd != rhs);
}

bool WTimer::Start(long time_value, long interval)
{
    if (this->_active)
    {
        return false;
    }

    this->_fd = CreateNewTimerfd();
    if (this->_fd == -1)
    {
        std::cout << "new timerfd error" << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "timerfd created successfully" << std::endl;
    
    struct itimerspec next_time{0};
    next_time.it_value.tv_sec = time_value / 1000L;
    next_time.it_value.tv_nsec = (time_value % 1000L) * 1000'000L;
    next_time.it_interval.tv_sec = interval / 1000L;
    next_time.it_interval.tv_nsec = (interval % 1000L) * 1000'000L;
    if (!SetTimerTime(this->_fd, SetTimeFlag::REL, &next_time))
    {
        std::cout << "SetTimerTime failed" << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "SetTimerTime succ" << std::endl;

    this->_handler->AddTimer(this->_listener, this->_fd);

    return true;
}



}   // namespace wlb
#endif
