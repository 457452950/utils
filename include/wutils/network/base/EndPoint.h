#pragma once
#ifndef UTIL_ENDPOINT_H
#define UTIL_ENDPOINT_H

#include <cstdint>
#include <string>

#include <netinet/in.h> // sockaddr

#include "wutils/network/Tools.h"
#include "wutils/network/base/Address.h"

namespace wutils::network {

template <class FAMILY>
bool IpAddrToStr(const typename FAMILY::in_addr &addr, std::string &buf) {
    static int _len = 150;
    auto       temp = std::make_unique<char[]>(_len);
    if(::inet_ntop(FAMILY::Family(), (void *)&addr, temp.get(), _len) == nullptr) {
        buf.clear();
        return false;
    }

    buf.assign(temp.get());
    return true;
}

template <class FAMILY>
bool IpStrToAddr(const std::string &ip_str, typename FAMILY::in_addr *addr) {
    if(::inet_pton(FAMILY::Family(), ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

template <class FAMILY>
class IEndPointInfo {
public:
    IEndPointInfo() = default;

    using Family = FAMILY;

    operator bool() const noexcept { return this->is_valid_; }

    bool Assign(const typename FAMILY::native_sockaddr_type &addr) {
        memcpy(this->sockaddr_.data(), (void *)&addr, FAMILY::sockaddr_len);
        is_valid_ = true;
        return is_valid_;
    };
    const typename FAMILY::native_sockaddr_type *AsSockAddr() const {
        return (typename FAMILY::native_sockaddr_type *)sockaddr_.data();
    };
    typename FAMILY::native_sockaddr_type *AsSockAddr() {
        return (typename FAMILY::native_sockaddr_type *)sockaddr_.data();
    };
    int GetSockAddrLen() const { return FAMILY::sockaddr_len; }

protected:
    typename FAMILY::SockAddr sockaddr_;
    bool                      is_valid_{false};
};

} // namespace wutils::network

#endif // UTIL_ENDPOINT_H
