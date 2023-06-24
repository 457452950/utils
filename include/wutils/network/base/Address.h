#pragma once
#ifndef UTIL_ADDRESS_H
#define UTIL_ADDRESS_H

#include "wutils/network/Tools.h"

namespace wutils::network::ip {

template <class FAMILY>
class IAddress {
public:
    IAddress() = default;
    explicit IAddress(const std::string &ip) { is_valid_ = IpStrToAddr(ip, &this->in_addr_); }
    virtual ~IAddress() = default;

    using Family = FAMILY;

    bool Assign(const std::string &ip) {
        is_valid_ = IpStrToAddr(ip, &this->in_addr_);
        return is_valid_;
    };
    bool Assign(const typename FAMILY::native_data_type &in) {
        this->in_addr_ = in;
        is_valid_      = true;
        return is_valid_;
    }

    operator bool() const { return is_valid_; }

    typename FAMILY::native_data_type AsInAddr() const { return this->in_addr_; };
    std::string                       AsString() const {
        std::string str;
        if(!IpAddrToStr(this->in_addr_, str)) {
            return "";
        }
        return str;
    }

private:
    typename FAMILY::native_data_type in_addr_{0};
    bool                              is_valid_{false};
};

} // namespace wutils::network::ip

#endif // UTIL_ADDRESS_H
