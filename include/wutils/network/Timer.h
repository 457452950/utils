#pragma once
#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include "Socket.h"
#include "Tools.h"

namespace wutils::network {
namespace timer {

class Socket : public SOCKET {
public:
    Socket() : SOCKET(CreateNewTimerFd()){};
    Socket(const Socket &other) : SOCKET(other) {}
    ~Socket() override = default;

    void Read() {
        uint64_t exp = 0;
        ::read(this->socket_, &exp, sizeof(exp));
    }
};

} // namespace timer

} // namespace wutils::network


#endif // UTIL_TIMER_H
