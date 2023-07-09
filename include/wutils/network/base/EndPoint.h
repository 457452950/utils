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

    using Family     = FAMILY;
    using sockaddr_t = typename FAMILY::sockaddr_t;
    using data_t     = std::array<uint8_t, FAMILY::sockaddr_len>;

    operator bool() const noexcept { return this->is_valid_; }

    bool Assign(const sockaddr_t &addr) {
        memcpy(this->sockaddr_.data(), (void *)&addr, FAMILY::sockaddr_len);
        is_valid_ = true;
        return is_valid_;
    };
    const sockaddr_t *AsSockAddr() const { return (sockaddr_t *)sockaddr_.data(); };
    sockaddr_t       *AsSockAddr() { return (sockaddr_t *)sockaddr_.data(); };

    int GetSockAddrLen() const { return FAMILY::sockaddr_len; }

protected:
    data_t sockaddr_;
    bool   is_valid_{false};
};

} // namespace wutils::network

#endif // UTIL_ENDPOINT_H
