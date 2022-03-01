#pragma once

#include <string>
#include <stdint.h>
#include <exception>

#include "../WOS.h"
#ifdef OS_IS_WINDOWS

#elif defined OS_IS_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>    // tcp_nodelay
#include <arpa/inet.h>      // inet_ntop inet_pton
#include <cstring>          // strerror
#include <cerrno>
#include <fcntl.h>          // fcntl

#endif

namespace wlb
{
namespace NetWork
{

using base_socket_type = int32_t;
using base_socket_ptr = base_socket_type*;


enum class AF_FAMILY 
{
    INET = AF_INET,
    INET6 = AF_INET6
};

struct WEndPointInfo;

// sockaddr
// sockaddr_in sockaddr_in6

// in_addr  in6_addr

bool IpAddrToString(in_addr addr, std::string& buf);
bool IpAddrToString(in6_addr addr, std::string& buf);

bool StringToIpAddress(const std::string& ip_str, in_addr& addr);
bool StringToIpAddress(const std::string& ip_str, in6_addr& addr);

bool HtoNS(const uint16_t host_num, uint16_t& net_num);
bool NtoHS(const uint16_t net_num, uint16_t& host_num);


bool MakeSockAddr_in(const std::string& ip_address, uint16_t port, sockaddr_in& addr);


// server methods
bool Bind(base_socket_type socket, const std::string& host, uint16_t port, bool isv4 = true);
bool Bind(base_socket_type socket, const WEndPointInfo& serverInfo);

base_socket_type Accept(base_socket_type socket, WEndPointInfo& info, bool isv4 = true);
base_socket_type AcceptV4(base_socket_type socket, WEndPointInfo& info);

bool ConnectToHost(base_socket_type socket, const std::string& host, uint16_t port, bool isv4 = true);
bool ConnectToHost(base_socket_type socket, const WEndPointInfo& info);

// socket function
bool SetSocketNoBlock(base_socket_type socket);
bool SetSocketReuseAddr(base_socket_type socket);
bool SetSocketReusePort(base_socket_type socket);
bool SetSocketKeepAlive(base_socket_type socket);


// tcp socket function
bool SetTcpSocketNoDelay(base_socket_type socket);


// IP + port + isv4
struct WEndPointInfo
{
    std::string ip_address;
    uint16_t port;
    bool isv4;
    WEndPointInfo(const std::string& _address = "", uint16_t _port = 0, bool _isv4 = true)
        : ip_address(_address), port(_port), isv4(_isv4)
    {;};
    static const WEndPointInfo FromNet(const sockaddr_in& net) {
        WEndPointInfo info;
        if ( !IpAddrToString(net.sin_addr, info.ip_address))
        {
            return info;
        }
        if (!wlb::NetWork::NtoHS(net.sin_port, info.port))
        {
            return info;
        }
        return info;
    }
    // static const WEndPointInfo FromNet(const sockaddr_in6& net) {
    //     WEndPointInfo info;
    //     return info;
    // }
    bool ToNet(sockaddr_in& _sockaddr_in) {
        if (!this->isv4){
            return false;
        }

        _sockaddr_in.sin_family = AF_INET;
        uint16_t h_port = 0;
        in_addr h_addr;

        if (!HtoNS(this->port, h_port))
        {
            return false;
        }
        if (!StringToIpAddress(this->ip_address, h_addr))
        {
            return false;
        }
        
        _sockaddr_in.sin_port = h_port;
        _sockaddr_in.sin_addr = h_addr;
        return true;
    }
};




}   // namespace NetWork
}   // namespace wlb
