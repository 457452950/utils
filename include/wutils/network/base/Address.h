#pragma once
#ifndef UTIL_ADDRESS_H
#define UTIL_ADDRESS_H

#include "wutils/network/Tools.h"

namespace wutils::network::ip {

template <class FAMILY>
class IAddress {
public:
    IAddress() = default;
    explicit IAddress(const std::string &ip) { is_valid_ = IpStrToAddr(ip, &this->address_); }
    virtual ~IAddress() = default;

    using Family    = FAMILY;
    using address_t = typename Family::address_t;

    bool Assign(const std::string &ip) {
        is_valid_ = IpStrToAddr(ip, &this->address_);
        return is_valid_;
    };
    bool Assign(const address_t &in) {
        this->address_ = in;
        is_valid_      = true;
        return is_valid_;
    }

    operator bool() const { return is_valid_; }

    address_t   AsInAddr() const { return this->address_; };
    std::string AsString() const {
        std::string str;
        if(!IpAddrToStr(this->address_, str)) {
            return "";
        }
        return str;
    }

private:
    address_t address_{0};
    bool      is_valid_{false};
};

} // namespace wutils::network::ip

#endif // UTIL_ADDRESS_H
