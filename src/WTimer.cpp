#include "WTimer.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <exception>

#if defined OS_IS_LINUX

namespace wlb {

timerfd CreateNewTimerfd() {
    // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    // return ::timerfd_create(CLOCK_REALTIME, 0);
}

bool SetTimerTime(timerfd fd,
                  SetTimeFlag flag,
                  const struct itimerspec *next_time,
                  struct itimerspec *prev_time) {
    if (::timerfd_settime(fd, (int) flag, next_time, prev_time) == 0) {
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////
// timer 

bool WTimer::Start(long time_value, long interval) {
    if (this->active_) {
        return false;
    }

    if (this->fd_ == -1) {
        this->fd_ = CreateNewTimerfd();
        if (this->fd_ == -1) {
            return false;
        }
    }

    struct itimerspec next_time{0};
    next_time.it_value.tv_sec     = time_value / 1000L;
    next_time.it_value.tv_nsec    = (time_value % 1000L) * 1000'000L;
    next_time.it_interval.tv_sec  = interval / 1000L;
    next_time.it_interval.tv_nsec = (interval % 1000L) * 1000'000L;
    if (!SetTimerTime(this->fd_, SetTimeFlag::REL, &next_time)) {
        return false;
    }

    if (this->timerHandlerData_ == nullptr) {
        this->timerHandlerData_ = new(std::nothrow) WTimerHandlerData(this, this->listener_, this->fd_);
        if (this->timerHandlerData_ == nullptr) {
            return false;
        }
    }

    this->timerHandler_->AddTimer(this->timerHandlerData_);

    this->active_ = true;

    return true;
}
void WTimer::Stop() {
    if (!this->active_) {
        return;
    }

    this->timerHandler_->RemoveTimer(this->timerHandlerData_);
    this->active_ = false;
}
WTimer::~WTimer() {
    if (this->fd_ != -1) {
        ::close(this->fd_);
    }
    this->fd_ = -1;

    delete this->timerHandlerData_;
    this->timerHandlerData_ = nullptr;
}

}   // namespace wlb
#endif
