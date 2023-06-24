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
    auto s = ::socket(FAMILY::Family(), PROTOCOL::Type(), PROTOCOL::Protocol());
    std::cout << "socket " << s << std::endl;
    return s;
}

class ISocket {
public:
    explicit ISocket(std::nullptr_t) {}
    explicit ISocket(int socket);
    ISocket(const ISocket &other);
    ~ISocket();

    ISocket &operator=(const ISocket &other);
    operator bool() const noexcept { return socket_ != -1; }
    socket_t GetNativeSocket() const { return socket_; }


protected:
    int32_t *flag_{nullptr};
    socket_t socket_{-1};
};

ISocket::ISocket(int socket) : socket_(socket) {
    if(flag_ == nullptr) {
        flag_ = new int32_t;
    }
    *flag_ = 1;
}

ISocket::ISocket(const ISocket &other) {
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

ISocket::~ISocket() {
    if(flag_) {
        --(*flag_);
        if(*flag_ == 0) {
            std::cout << "close " << this->socket_ << std::endl;
            ::close(socket_);
        }
    }
}

ISocket &ISocket::operator=(const ISocket &other) {
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
