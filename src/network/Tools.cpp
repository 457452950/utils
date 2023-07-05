#include "wutils/network/Tools.h"

#include <cassert>
#include <iostream>

#include "wutils/network/EndPoint.h"

namespace wutils::network {

timerfd_t CreateNewTimerfd() {
    // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
    //    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    return ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    // return ::timerfd_create(CLOCK_REALTIME, 0);
}

bool SetTimerTime(timerfd_t fd, SetTimeFlag flag, const struct itimerspec *next_time, struct itimerspec *prev_time) {
    if(::timerfd_settime(fd, (int)flag, next_time, prev_time) == 0) {
        return true;
    }
    return false;
}

// -1 for errno
socket_t MakeSocket(enum AF_FAMILY family, enum AF_PROTOL protol) {
    if(protol == AF_PROTOL::TCP) {
        return ::socket(static_cast<int>(family), SOCK_STREAM, IPPROTO_TCP);
    } else {
        return ::socket(static_cast<int>(family), SOCK_DGRAM, IPPROTO_UDP);
    }
}
socket_t MakeTcpV4Socket() { return MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP); }
socket_t MakeUdpV4Socket() { return MakeSocket(AF_FAMILY::INET, AF_PROTOL::UDP); }

bool IpAddrToString(in_addr addr_, std::string &buf) {
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

bool IpAddrToString(in6_addr addr, std::string &buf) {
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

bool IPStringToAddress(const std::string &ip_str, in_addr *addr) {
    if(::inet_pton(AF_INET, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

bool IPStringToAddress(const std::string &ip_str, in6_addr *addr) {
    std::cout << "IPv6StringToAddress " << ip_str << std::endl;
    if(::inet_pton(AF_INET6, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

bool HtoNS(uint16_t host_num, uint16_t *net_num) {
    *net_num = ::htons(host_num);
    return true;
}
bool NtoHS(uint16_t net_num, uint16_t *host_num) {
    *host_num = ::ntohs(net_num);
    return true;
}

bool MakeSockAddr_in(const std::string &ip_address, uint16_t port, sockaddr_in *addr) {
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
bool MakeSockAddr_in6(const std::string &ip_address, uint16_t port, sockaddr_in6 *addr) {
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

bool Bind(socket_t socket, const EndPointInfo &serverInfo) {
    auto ok = ::bind(socket, serverInfo.GetSockAddr(), serverInfo.GetSockSize());
    if(ok == 0) {
        return true;
    }

    return false;
}


socket_t MakeBindedSocket(const EndPointInfo &info) {
    socket_t bind_sock = INVALID_SOCKET;
    auto     fami      = info.GetFamily();

    bind_sock = MakeSocket(fami, AF_PROTOL::UDP);
    if(bind_sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    if(Bind(bind_sock, info)) {
        return bind_sock;
    }

    ::close(bind_sock);
    return INVALID_SOCKET;
}

socket_t MakeListenedSocket(const EndPointInfo &info) {
    socket_t listen_sock(INVALID_SOCKET);
    auto     fami = info.GetFamily();

    listen_sock = MakeSocket(fami, AF_PROTOL::TCP);
    if(listen_sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    if(Bind(listen_sock, info)) {
        if(::listen(listen_sock, 1024) == 0) {
            return listen_sock;
        }
    }

    ::close(listen_sock);
    return INVALID_SOCKET;
}

socket_t Accept(socket_t socket, EndPointInfo &info) {
    sockaddr_in6 temp{};
    socklen_t    len{sizeof(temp)};
    socket_t     _client_sock = ::accept(socket, (sockaddr *)&temp, &len);
    if(_client_sock < 0) {
        return INVALID_SOCKET;
    }
    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return _client_sock;
}

socket_t Accept4(socket_t socket, EndPointInfo &info, int flags) {
    sockaddr_in6 temp{};
    socklen_t    len{sizeof(temp)};
    socket_t     _client_sock = ::accept4(socket, (sockaddr *)&temp, &len, flags);
    if(_client_sock < 0) {
        return INVALID_SOCKET;
    }
    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return _client_sock;
}


socket_t ConnectToHost(const EndPointInfo &info, AF_PROTOL protol) {
    auto lis_sock = MakeSocket(info.GetFamily(), protol);
    if(lis_sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    return ConnectToHost(lis_sock, info);
}

bool ConnectToHost(socket_t socket, const EndPointInfo &info) {
    if(::connect(socket, info.GetSockAddr(), info.GetSockSize()) == 0) {
        return true;
    }

    return false;
}

/***************************************************
 * UDPPointer Utils
 ****************************************************/

int64_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, EndPointInfo &info) {
    socklen_t    len = 0;
    sockaddr_in6 temp{0};

    auto recv_len = ::recvfrom(socket, buf, buf_len, 0, (sockaddr *)&temp, &len);
    if(recv_len < 0) {
        return -1;
    }

    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return recv_len;
}

/***************************************************
 * Socket Utils
 ****************************************************/
bool IsSocketNonBlock(socket_t socket) { return GetSocketFlags(socket) & O_NONBLOCK; }

bool SetSocketNonBlock(socket_t socket, bool is_set) {
    switch(is_set) {
    case true:
        return ::fcntl(socket, F_SETFL, GetSocketFlags(socket) | O_NONBLOCK) == 0;
    case false:
        return ::fcntl(socket, F_SETFL, GetSocketFlags(socket) & (~O_NONBLOCK)) == 0;
    }
}

bool SetSocketReuseAddr(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == 0;
}

bool SetSocketReusePort(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == 0;
}
bool SetSocketKeepAlive(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == 0;
}

bool SetTcpSocketNoDelay(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt, len) == 0;
}

bool GetSockName(socket_t socket, EndPointInfo &info) {
    sockaddr_in6 sa{};
    socklen_t    len = v6::SOCKADDR_LEN;

    auto ok = ::getsockname(socket, reinterpret_cast<sockaddr *>(&sa), &len) == 0;
    if(ok) {
        info.Assign((sockaddr *)&sa, len);
    }

    return ok;
}
bool GetPeerName(socket_t socket, EndPointInfo &info) {
    sockaddr_in6 sa{};
    socklen_t    len = v6::SOCKADDR_LEN;

    auto ok = ::getpeername(socket, reinterpret_cast<sockaddr *>(&sa), &len) == 0;
    if(ok) {
        info.Assign((sockaddr *)&sa, len);
    }

    return ok;
}


} // namespace wutils::network
