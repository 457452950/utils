#pragma once
#ifndef UTIL_ENDPOINT_H
#define UTIL_ENDPOINT_H

#include <cstdint>
#include <string>

#include <netinet/in.h> // sockaddr

#include "wutils/network/Tools.h"
#include "wutils/network/base/Address.h"

namespace wutils::network::ip {

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
class ENDPOINTINFO {
public:
    ENDPOINTINFO() = default;

    bool            Assign(const typename FAMILY::addr &addr);
    const sockaddr *AsSockAddr() const { return (sockaddr *)addr_.data(); };
    sockaddr       *AsSockAddr() { return (sockaddr *)addr_.data(); };
    int             GetSockAddrLen() const { return FAMILY::addr_len; }

    using Family = FAMILY;

protected:
    typename FAMILY::Addr addr_;
};

template <class FAMILY>
bool ENDPOINTINFO<FAMILY>::Assign(const typename FAMILY::addr &addr) {
    memcpy(this->addr_.data(), (void *)&addr, sizeof(FAMILY::addr));
    return true;
}

} // namespace wutils::network::ip

#endif // UTIL_ENDPOINT_H
