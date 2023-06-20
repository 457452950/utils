#pragma once
#ifndef UTIL_SOCKET_H
#define UTIL_SOCKET_H

#include <cstdint>
#include <unistd.h> // close(int)

#include <netinet/in.h>

#include "NetworkDef.h"

namespace wutils::network {

template <class FAMILY, class PROTOCOL>
socket_t CreateSocket() {
    return ::socket(FAMILY::Family(), PROTOCOL::Type(), PROTOCOL::Protocol());
}

class SOCKET {
public:
    explicit SOCKET(std::nullptr_t) {}
    explicit SOCKET(int socket);
    SOCKET(const SOCKET &other);
    virtual ~SOCKET();

    SOCKET &operator=(const SOCKET &other);
    operator bool() const noexcept { return socket_ != -1; }
    socket_t GetNativeSocket() const { return socket_; }

protected:
    socket_t socket_{-1};
    int32_t *flag_{nullptr};
};

SOCKET::SOCKET(int socket) : socket_(socket) {
    if(flag_ == nullptr) {
        flag_ = new int32_t;
    }
    *flag_ = 1;
}

SOCKET::SOCKET(const SOCKET &other) {
    if(flag_ != nullptr) {
        --(*flag_);
        if(*flag_ == 0) {
            ::close(socket_);
        }
    }

    if(other) {
        this->flag_   = other.flag_;
        this->socket_ = other.socket_;
        ++(*flag_);
    } else {
        this->socket_ = -1;
        this->flag_   = nullptr;
    }
}

SOCKET::~SOCKET() {
    --(*flag_);
    if(*flag_ == 0) {
        ::close(socket_);
    }
}
SOCKET &SOCKET::operator=(const SOCKET &other) {
    if(this == &other) {
        return *this;
    }

    if(flag_ != nullptr) {
        --(*flag_);
        if(*flag_ == 0) {
            ::close(socket_);
        }
    }

    if(other) {
        this->flag_   = other.flag_;
        this->socket_ = other.socket_;
        ++(*flag_);
    } else {
        this->socket_ = -1;
        this->flag_   = nullptr;
    }
    return *this;
}

} // namespace wutils::network

#endif // UTIL_SOCKET_H
