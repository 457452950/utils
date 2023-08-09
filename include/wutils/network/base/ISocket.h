#pragma once
#ifndef UTIL_BASE_ISOCKET_H
#define UTIL_BASE_ISOCKET_H

#include <unistd.h> // close(int)

#include "Native.h"

namespace wutils::network {

class ISocket {
public:
    ISocket() = default;

    ISocket(socket_t socket) : socket_(socket) {}

    ISocket(const ISocket &other) = default;

    ~ISocket() = default;

    ISocket &operator=(const ISocket &other) {
        if(this != &other) {
            this->socket_ = other.socket_;
        }
        return *this;
    }

    explicit operator bool() const { return this->socket_ != INVALID_SOCKET; }

    socket_t Get() const { return socket_; }

    virtual void Close() {
        if(socket_ != INVALID_SOCKET) {
            ::close(this->socket_);
            this->socket_ = INVALID_SOCKET;
        }
    }

protected:
    socket_t socket_{INVALID_SOCKET};
};

} // namespace wutils::network

#endif // UTIL_BASE_ISOCKET_H
