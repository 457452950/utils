#pragma once
#ifndef UTIL_ISOCKET_H
#define UTIL_ISOCKET_H

#include <cstdint>
#include <unistd.h> // close(int)

#include "Defined.h"

namespace wutils::network {

template <class FAMILY, class PROTOCOL>
socket_t CreateSocket() {
    auto s = ::socket(FAMILY::Family(), PROTOCOL::Type(), PROTOCOL::Protocol());
    return s;
}

class ISocket {
public:
    explicit ISocket(std::nullptr_t) {}
    explicit ISocket(int socket) : socket_(socket) {}
    ISocket(const ISocket &other) = default;
    ~ISocket()                    = default;

    ISocket &operator=(const ISocket &other) = default;

    operator bool() const noexcept { return socket_ != INVALID_SOCKET; }

    socket_t GetNativeSocket() const { return socket_; }

    void Close() {
        if(socket_ != INVALID_SOCKET) {
            ::close(this->socket_);
            this->socket_ = INVALID_SOCKET;
        }
    }

protected:
    socket_t socket_{INVALID_SOCKET};
};


} // namespace wutils::network

#endif // UTIL_ISOCKET_H
