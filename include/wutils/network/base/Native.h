#pragma once
#ifndef UTILS_NETWORK_DEF_H
#define UTILS_NETWORK_DEF_H

#include <iostream>

#include <arpa/inet.h>  // inet_ntop inet_pton
#include <netinet/in.h> // AF_INET IPPROTO_TCP

#include "wutils/base/HeadOnly.h"

namespace wutils::network {

enum AF_FAMILY { INET = AF_INET, INET6 = AF_INET6, UNIX = AF_UNIX };
enum AF_PROTOL { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

namespace v4 {
constexpr int FAMILY       = AF_FAMILY::INET;
using SockAddr             = ::sockaddr_in;
constexpr int SOCKADDR_LEN = sizeof(sockaddr_in);

constexpr char const *LOOPBACK = "127.0.0.1";
constexpr char const *ANY      = "0.0.0.0";
} // namespace v4
namespace v6 {
constexpr int FAMILY       = AF_FAMILY::INET6;
using SockAddr             = ::sockaddr_in6;
constexpr int SOCKADDR_LEN = sizeof(sockaddr_in6);

constexpr char const *ANY = "0:0:0:0:0:0:0:0";
} // namespace v6

using socket_t = int32_t;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

using timerfd_t = int32_t;


// INVALID_SOCKET for errno
HEAD_ONLY socket_t MakeSocket(enum AF_FAMILY family, AF_PROTOL protol) {
    if(protol == AF_PROTOL::TCP) {
        return ::socket(static_cast<int>(family), SOCK_STREAM, IPPROTO_TCP);
    } else {
        return ::socket(static_cast<int>(family), SOCK_DGRAM, IPPROTO_UDP);
    }
}
HEAD_ONLY socket_t MakeTcpV4Socket() { return MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP); }
HEAD_ONLY socket_t MakeTcpV6Socket() { return MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP); }
HEAD_ONLY socket_t MakeUdpV4Socket() { return MakeSocket(AF_FAMILY::INET, AF_PROTOL::UDP); }
HEAD_ONLY socket_t MakeUdpV6Socket() { return MakeSocket(AF_FAMILY::INET6, AF_PROTOL::UDP); }


/***************************************************
 * IP Port Utils
 ****************************************************/

HEAD_ONLY bool IpAddrToString(in_addr addr_, std::string &buf) {
    try {
        constexpr int32_t _len = 50;
        char             *_buf = new char[_len]{0};
        if(::inet_ntop((int)AF_FAMILY::INET, (void *)&addr_, _buf, _len) == nullptr) {
            buf.clear();
            return false;
        }

        buf.assign(_buf);
        delete[] _buf;
        return true;
    } catch(const std::exception &e) {
        std::cerr << "IpAddrToString fail, error : " << e.what() << std::endl;
        return false;
    }
}
HEAD_ONLY bool IpAddrToString(in6_addr addr, std::string &buf) {
    try {
        constexpr int32_t _len = 100;
        char             *_buf = new char[_len];
        if(::inet_ntop((int)AF_FAMILY::INET6, (void *)&addr, _buf, _len) == nullptr) {
            buf.clear();
            return false;
        }

        buf.assign(_buf);
        delete[] _buf;
        return true;
    } catch(const std::exception &e) {
        std::cerr << "IpAddrToString fail, error : " << e.what() << std::endl;
        return false;
    }
}

HEAD_ONLY bool IPStringToAddress(const std::string &ip_str, in_addr *addr) {
    if(::inet_pton(AF_INET, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}
HEAD_ONLY bool IPStringToAddress(const std::string &ip_str, in6_addr *addr) {
    std::cout << "IPv6StringToAddress " << ip_str << std::endl;
    if(::inet_pton(AF_INET6, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

HEAD_ONLY bool HtoNS(uint16_t host_num, uint16_t *net_num) {
    *net_num = ::htons(host_num);
    return true;
}
HEAD_ONLY bool NtoHS(uint16_t net_num, uint16_t *host_num) {
    *host_num = ::ntohs(net_num);
    return true;
}

HEAD_ONLY bool MakeSockAddr_in(const std::string &ip_address, uint16_t port, sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    uint16_t h_port  = 0;
    in_addr  h_addr{};

    HtoNS(port, &h_port);

    if(!IPStringToAddress(ip_address, &h_addr)) {
        std::cout << "MakeSockAddr_in fail " << ip_address << std::endl;
        return false;
    }

    addr->sin_port = h_port;
    addr->sin_addr = h_addr;
    return true;
}
HEAD_ONLY bool MakeSockAddr_in6(const std::string &ip_address, uint16_t port, sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;
    uint16_t h_port   = 0;
    in6_addr h_addr{};

    if(!HtoNS(port, &h_port)) {
        return false;
    }

    if(!IPStringToAddress(ip_address, &h_addr)) {
        return false;
    }

    addr->sin6_port     = h_port;
    addr->sin6_addr     = h_addr;
    addr->sin6_flowinfo = 0;
    return true;
}


} // namespace wutils::network


#endif // UTILS_NETWORK_DEF_H
