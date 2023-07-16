#pragma once
#ifndef UTIL_EASY_TIMER_H
#define UTIL_EASY_TIMER_H

#include "../base/ISocket.h"
#include "Tools.h"

namespace wutils::network::timer {

class Socket : public ISocket {
public:
    Socket() : ISocket(CreateNewTimerfd()) {}
    ~Socket() = default;

    Socket(const Socket &other) = default;

    Socket &operator=(const Socket &other) = default;

    // Common
    uint64_t Read() {
        uint64_t exp = 0;
        return ::read(this->socket_, &exp, sizeof(exp));
    }

    bool SetTimeOut(const ::itimerspec *next_time, ::itimerspec *prev_time = nullptr) {
        return SetTimerTimeOut(this->socket_, SetTimeFlag::REL, next_time, prev_time);
    }

    // Socket optional
    bool IsNonBlock() { return IsSocketNonBlock(this->socket_); }
    bool SetNonBlock(bool is_set) { return SetSocketNonBlock(this->socket_, is_set); }
};

} // namespace wutils::network::timer

#endif // UTIL_EASY_TIMER_H
