#pragma once
#ifndef WUTILS_NET_DNS_DNS_H
#define WUTILS_NET_DNS_DNS_H

#include <future>
#include <list>
#include <netdb.h> // gnu

#include "wutils/network/NetAddress.h"
#include "Error.h"


namespace wutils::net::dns {

struct AddrInfo {
    explicit AddrInfo(addrinfo *info) {
        net_address.Assign(info->ai_addr, info->ai_addrlen);
        socket_type = info->ai_socktype;
        protocol    = info->ai_protocol;
        if(info->ai_canonname)
            name = info->ai_canonname;
    }

    int                 socket_type;
    int                 protocol;
    std::string         name;
    network::NetAddress net_address;
};

class Result {
public:
    explicit Result(const std::string &host, int error) : request_host_(host) { error_ = dns::make_error_code(error); }
    explicit Result(const std::string &host, addrinfo *info) : request_host_(host) {
        if(info == nullptr) {
            return;
        }

        while(info != nullptr) {
            data_.emplace_back(info);
            info = info->ai_next;
        }
    }

    bool                       Success() const { return !data_.empty(); }
    const std::list<AddrInfo> &Get() const { return data_; }
    const Error               &GetError() const { return error_; }

private:
    std::string         request_host_;
    std::list<AddrInfo> data_;
    Error               error_;
};

enum Family { INET = AF_INET, INET6 = AF_INET6, BOTH = 0 };
enum Protocol { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP, ALL = 0 };

class DNS {
public:
    static std::future<Result> Request(const std::string &host, Family family = BOTH, Protocol procol = ALL) {
        return std::async([=]() -> Result {
            addrinfo  input_info{0};
            addrinfo *result;

            input_info.ai_family   = family;
            input_info.ai_flags    = AI_CANONNAME;
            input_info.ai_protocol = procol;

            int err = ::getaddrinfo(host.c_str(), nullptr, &input_info, &result);
            if(err != 0) {
                // false
                return Result{host, err};
            }

            Result res{host, result};
            freeaddrinfo(result);

            return res;
        });
    }
}; // namespace wutils::net::dns


} // namespace wutils::net::dns

#endif // WUTILS_NET_DNS_DNS_H
