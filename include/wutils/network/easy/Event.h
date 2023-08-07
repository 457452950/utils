#pragma once
#ifndef UTIL_EVENT_H
#define UTIL_EVENT_H

#include <sys/eventfd.h>

#include "wutils/network/base/ISocket.h"

namespace wutils::network::event {

using Value = eventfd_t;

class Socket : public ISocket {
public:
    Socket() : ISocket(eventfd(0, 0)) {}
    ~Socket() = default;

    Socket(const Socket &other) = default;
    explicit Socket(const ISocket &other) { this->socket_ = other.Get(); };

    using ISocket::operator=;
    Socket &operator=(const Socket &other) = default;

    using ISocket::operator bool;

    int   Write(Value value) { return eventfd_write(this->socket_, value); }
    Value Read() {
        Value va;
        eventfd_read(this->socket_, &va);
        return va;
    }
};


} // namespace wutils::network::event

#endif // UTIL_EVENT_H
