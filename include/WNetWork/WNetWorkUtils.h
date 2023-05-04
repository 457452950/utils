#pragma once
#ifndef UTILS_WNETWORK_UTILS_H
#define UTILS_WNETWORK_UTILS_H

#include <cerrno>
#include <cstdint>
#include <cstring> // strerror
#include <exception>
#include <fcntl.h> // fcntl
#include <memory>
#include <string>
#include <tuple>
#include <unistd.h>
#include <unordered_map> //std::hash

#include <sys/socket.h>
#include <sys/timerfd.h>

#include <arpa/inet.h> // inet_ntop inet_pton
#include <netinet/in.h>
#include <netinet/tcp.h> // tcp_nodelay

#include "../WOS.h"


namespace wlb::network {

int         GetError();
const char *ErrorToString(int error);

enum AF_FAMILY { INET = AF_INET, INET6 = AF_INET6 };
enum AF_PROTOL { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

struct WEndPointInfo;

using socket_t = int32_t;
using socket_p = socket_t *;

using timerfd_t = int32_t;
using timerfd_p = timerfd_t *;

// 设置时间差的意义
enum class SetTimeFlag {
    REL = 0, // 相对时间
    ABS = 1, // 绝对时间
};

timerfd_t CreateNewTimerfd();
bool      SetTimerTime(timerfd_t                fd,
                       SetTimeFlag              flag,
                       const struct itimerspec *next_time,
                       struct itimerspec       *prev_time = nullptr);


// sockaddr
// sockaddr_in      sockaddr_in6
// in_addr          in6_addr

socket_t MakeSocket(enum AF_FAMILY family, enum AF_PROTOL protol);
socket_t MakeTcpV4Socket();
socket_t MakeUdpV4Socket();


/***************************************************
 * IP Port Utils
 ****************************************************/

bool IpAddrToString(in_addr addr, std::string &buf);
bool IpAddrToString(in6_addr addr, std::string &buf);

bool IPStringToAddress(const std::string &ip_str, in_addr *addr);
bool IPStringToAddress(const std::string &ip_str, in6_addr *addr);

bool HtoNS(uint16_t host_num, uint16_t *net_num);
bool NtoHS(uint16_t net_num, uint16_t *host_num);

bool MakeSockAddr_in(const std::string &ip_address, uint16_t port, sockaddr_in *addr);
bool MakeSockAddr_in6(const std::string &ip_address, uint16_t port, sockaddr_in6 *addr);

// server methods
// bool Bind(socket_t socket, const std::string &host, uint16_t port, bool isv4 = true);
bool Bind(socket_t socket, const WEndPointInfo &serverInfo);

socket_t MakeBindedSocket(const WEndPointInfo &info, bool reuse);
socket_t MakeListenedSocket(const WEndPointInfo &info, bool reuse);

/***************************************************
 * TCP Utils
 ****************************************************/
// return -1 if fail
socket_t Accept(socket_t socket, WEndPointInfo& info);
socket_t Accept4(socket_t socket, WEndPointInfo& info, int flags);

// tcp is default and return -1 if fail     
socket_t ConnectToHost(const WEndPointInfo &info, AF_PROTOL = AF_PROTOL::TCP);
bool ConnectToHost(socket_t socket, const WEndPointInfo &info);

/***************************************************
 * UDP Utils
 ****************************************************/

int32_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, WEndPointInfo& info);

/***************************************************
 * Socket Utils
 ****************************************************/

// socket function
bool SetSocketNoBlock(socket_t socket);
bool SetSocketReuseAddr(socket_t socket);
bool SetSocketReusePort(socket_t socket);
bool SetSocketKeepAlive(socket_t socket);

// tcp socket function
bool SetTcpSocketNoDelay(socket_t socket);

// IP + port + isv4
// struct WEndPointInfo {
//     std::string ip_address;
//     uint16_t    port;
//     bool        isv4;
//     WEndPointInfo(const std::string &_address = "", uint16_t _port = 0, bool _isv4 = true);

//     static WEndPointInfo FromNet(const sockaddr_in &net);
//     static WEndPointInfo FromNet(const sockaddr_in6 &net);
//     bool                 ToNet4(sockaddr_in *_sockaddr_in);
//     bool                 ToNet6(sockaddr_in6 *_socketaddr_in6);
// };


// IP + port + family
struct WEndPointInfo {

    static WEndPointInfo *MakeWEndPointInfo(const std::string &address, uint16_t port, AF_FAMILY family);

    bool Assign(const std::string &address, uint16_t port, AF_FAMILY family);
    bool Assign(const sockaddr *sock, AF_FAMILY family);

    static WEndPointInfo *Emplace(const sockaddr *addr, AF_FAMILY family);

    const sockaddr *GetAddr() const { return (sockaddr *)&addr_; }
    AF_FAMILY       GetFamily() const { return family_; }
    unsigned long   GetSockSize() const {
        return family_ == AF_FAMILY::INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    }

    static std::tuple<std::string, uint16_t> Dump(const WEndPointInfo &);

private:
    AF_FAMILY family_;
    uint8_t   addr_[sizeof(sockaddr_in6)]{0};

private:
    /*
             * Hash for IPv4
             *
             0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |              PORT             |             IP                |
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |              IP               |                           |F|P|
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *
             * Hash for IPv6
             *
             0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |              PORT             | IP[0] ^  IP[1] ^ IP[2] ^ IP[3]|
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |IP[0] ^  IP[1] ^ IP[2] ^ IP[3] |          IP[0] >> 16      |F|P|
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             */
    void SetHash() {

        switch(this->family_) {
        case AF_INET: {
            auto *SockAddrIn = reinterpret_cast<const struct sockaddr_in *>(&this->addr_);

            const uint64_t address = ntohl(SockAddrIn->sin_addr.s_addr);
            const uint64_t port    = (ntohs(SockAddrIn->sin_port));

            this->hash = port << 48;
            this->hash |= address << 16;
            this->hash |= 0x0000; // AF_INET.

            break;
        }

        case AF_INET6: {
            auto *sockAddrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(&this->addr_);
            auto *a           = reinterpret_cast<const uint32_t *>(std::addressof(sockAddrIn6->sin6_addr));

            const auto     address1 = a[0] ^ a[1] ^ a[2] ^ a[3];
            const auto     address2 = a[0];
            const uint64_t port     = ntohs(sockAddrIn6->sin6_port);

            this->hash = port << 48;
            this->hash |= static_cast<uint64_t>(address1) << 16;
            this->hash |= address2 >> 16 & 0xFFFC;
            this->hash |= 0x0002; // AF_INET6.

            break;
        }
        }

        // note:no safed protocol
        // Override least significant bit with protocol information:
        // - If UDP, start with 0.
        // - If TCP, start with 1.
        // if (this->protocol == AF_PROTOL::UDP)
        // {
        // 	this->hash |= 0x0000;
        // }
        // else
        // {
        // 	this->hash |= 0x0001;
        // }
    }

public:
    uint64_t hash{0u};
    bool     operator==(const WEndPointInfo &other) const noexcept { return this->hash == other.hash; }
    bool     operator!=(const WEndPointInfo &other) const noexcept { return this->hash != other.hash; }
};


} // namespace wlb::network

namespace std {
template <>
class hash<wlb::network::WEndPointInfo> {
public:
    size_t operator()(const wlb::network::WEndPointInfo &it) const noexcept { return it.hash; }
};
} // namespace std

#endif // UTILS_WNETWORK_UTILS_H