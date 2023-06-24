#pragma once
#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include <chrono>

#include "Socket.h"
#include "Tools.h"

namespace wutils::network::timer {

class Socket : public ISocket {
public:
    Socket() : ISocket(CreateNewTimerFd()){};
    Socket(const Socket &other) : ISocket(other) {}
    ~Socket() = default;

    template <typename Rep, typename Period>
    bool SetOnce(const std::chrono::duration<Rep, Period> &rtime) {
        if(rtime <= rtime.zero())
            return false;

        struct itimerspec next_time {
            {0, 0}, {0, 0},
        };

        auto s  = std::chrono::duration_cast<std::chrono::seconds>(rtime);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rtime - s);

        next_time.it_value.tv_sec  = s;
        next_time.it_value.tv_nsec = ns;

        if(!SetTimerTime(this->socket_, TimerFlag::REL, &next_time)) {
            return false;
        }

        return true;
    }

    template <typename Rep, typename Period>
    bool SetRepeat(const std::chrono::duration<Rep, Period> &rtime) {
        if(rtime <= rtime.zero())
            return false;

        struct itimerspec next_time {
            {0, 0}, {0, 0},
        };

        auto s  = std::chrono::duration_cast<std::chrono::seconds>(rtime);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rtime - s);

        next_time.it_value.tv_sec     = s;
        next_time.it_value.tv_nsec    = ns;
        next_time.it_interval.tv_sec  = s;
        next_time.it_interval.tv_nsec = ns;

        if(!SetTimerTime(this->socket_, TimerFlag::REL, &next_time)) {
            return false;
        }

        return true;
    }
    void SetTimePoint() {
        // TODO:
        abort();
    }

    void Read() {
        uint64_t exp = 0;
        ::read(this->socket_, &exp, sizeof(exp));
    }
};

} // namespace wutils::network::timer


#endif // UTIL_TIMER_H
