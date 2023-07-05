#pragma once
#ifndef UTILS_NETWORK_UTILS_H
#define UTILS_NETWORK_UTILS_H

#include <fcntl.h>  // fcntl
#include <string>
#include <unistd.h> // close(int)

#include <sys/timerfd.h>

#include <arpa/inet.h>   // inet_ntop inet_pton
#include <netinet/tcp.h> // tcp_nodelay

#include "Defined.h"

namespace wutils::network {


struct EndPointInfo;


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
bool Bind(socket_t socket, const EndPointInfo &serverInfo);

socket_t MakeBindedSocket(const EndPointInfo &info);
socket_t MakeListenedSocket(const EndPointInfo &info);

/***************************************************
 * TCP Utils
 ****************************************************/
// return -1 if fail
socket_t Accept(socket_t socket, EndPointInfo &info);
socket_t Accept4(socket_t socket, EndPointInfo &info, int flags);

// tcp is default and return -1 if fail
socket_t ConnectToHost(const EndPointInfo &info, AF_PROTOL = AF_PROTOL::TCP);
bool     ConnectToHost(socket_t socket, const EndPointInfo &info);

/***************************************************
 * UDP Utils
 ****************************************************/

int64_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, EndPointInfo &info);

/***************************************************
 * Socket Utils
 ****************************************************/

// socket function
inline int GetSocketFlags(socket_t socket) { return ::fcntl(socket, F_GETFL, 0); }

bool IsSocketNonBlock(socket_t socket);
bool SetSocketNonBlock(socket_t socket, bool is_set);
bool SetSocketReuseAddr(socket_t socket, bool is_set);
bool SetSocketReusePort(socket_t socket, bool is_set);
bool SetSocketKeepAlive(socket_t socket, bool is_set);

// tcp socket function
bool SetTcpSocketNoDelay(socket_t socket, bool is_set);

//
bool GetSockName(socket_t, EndPointInfo &info);
bool GetPeerName(socket_t, EndPointInfo &info);


} // namespace wutils::network

#endif // UTILS_NETWORK_UTILS_H