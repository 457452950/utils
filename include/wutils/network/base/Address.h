#pragma once
#ifndef UTIL_ADDRESS_H
#define UTIL_ADDRESS_H

#include "wutils/network/Tools.h"

namespace wutils::network::ip {

template <class FAMILY>
class ADDRESS {
public:
    ADDRESS() = default;
    explicit ADDRESS(const std::string &ip);
    virtual ~ADDRESS() = default;

    bool Assign(const std::string &ip);
    bool Assign(const typename FAMILY::in_addr &in);

    using Family = FAMILY;

    typename FAMILY::in_addr AsInAddr() const { return this->in_addr_; };
    std::string              AsString() const;

private:
    typename FAMILY::in_addr in_addr_{0};
};

template <class FAMILY>
bool ADDRESS<FAMILY>::Assign(const std::string &ip) {
    return IpStrToAddr(ip, &this->in_addr_);
}

template <class FAMILY>
bool ADDRESS<FAMILY>::Assign(const typename FAMILY::in_addr &in) {
    this->in_addr_ = in;
    return true;
}

template <class FAMILY>
ADDRESS<FAMILY>::ADDRESS(const std::string &ip) {
    IpStrToAddr(ip, &this->in_addr_);
}

template <class FAMILY>
std::string ADDRESS<FAMILY>::AsString() const {
    std::string str;
    if(!IpAddrToStr(this->in_addr_, str)) {
        return "";
    }
    return str;
}

} // namespace wutils::network::ip

#endif // UTIL_ADDRESS_H
