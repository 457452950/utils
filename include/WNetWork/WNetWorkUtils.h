#pragma once

#include <arpa/inet.h> // inet_ntop inet_pton
#include <cerrno>
#include <cstdint>
#include <cstring> // strerror
#include <exception>
#include <fcntl.h> // fcntl
#include <netinet/in.h>
#include <netinet/tcp.h> // tcp_nodelay
#include <string>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "../WOS.h"


namespace wlb::network {


enum class AF_FAMILY { INET = AF_INET, INET6 = AF_INET6 };
enum class AF_PROTOL { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

struct WEndPointInfo;
using base_socket_type = int32_t;
using base_socket_ptr  = base_socket_type *;

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

base_socket_type MakeSocket(enum AF_FAMILY family, enum AF_PROTOL protol);
base_socket_type MakeTcpV4Socket();
base_socket_type MakeUdpV4Socket();

bool IpAddrToString(in_addr addr, std::string *buf);
bool IpAddrToString(in6_addr addr, std::string *buf);

bool StringToIpAddress(const std::string &ip_str, in_addr *addr);
bool StringToIpAddress(const std::string &ip_str, in6_addr *addr);

bool HtoNS(uint16_t host_num, uint16_t *net_num);
bool NtoHS(uint16_t net_num, uint16_t *host_num);

bool MakeSockAddr_in(const std::string &ip_address, uint16_t port, sockaddr_in *addr);
bool MakeSockAddr_in6(const std::string &ip_address, uint16_t port, sockaddr_in6 *addr);

// server methods
bool Bind(base_socket_type socket, const std::string &host, uint16_t port, bool isv4 = true);
bool Bind(base_socket_type socket, const WEndPointInfo &serverInfo);

base_socket_type MakeListenedSocket(const WEndPointInfo &info, enum AF_PROTOL protol);

// return -1 if fail
base_socket_type Accept(base_socket_type socket, WEndPointInfo *info, bool isv4 = true);
base_socket_type AcceptV4(base_socket_type socket, WEndPointInfo *info);

bool ConnectToHost(base_socket_type socket, const std::string &host, uint16_t port, bool isv4 = true);
bool ConnectToHost(base_socket_type socket, const WEndPointInfo &info);

// socket function
bool SetSocketNoBlock(base_socket_type socket);
bool SetSocketReuseAddr(base_socket_type socket);
bool SetSocketReusePort(base_socket_type socket);
bool SetSocketKeepAlive(base_socket_type socket);

// tcp socket function
bool SetTcpSocketNoDelay(base_socket_type socket);

// IP + port + isv4
struct WEndPointInfo {
    std::string ip_address;
    uint16_t    port;
    bool        isv4;
    WEndPointInfo(const std::string &_address = "", uint16_t _port = 0, bool _isv4 = true);

    static WEndPointInfo FromNet(const sockaddr_in &net);
    static WEndPointInfo FromNet(const sockaddr_in6 &net);
    bool                 ToNet4(sockaddr_in *_sockaddr_in);
    bool                 ToNet6(sockaddr_in6 *_socketaddr_in6);
};

} // namespace wlb::network
