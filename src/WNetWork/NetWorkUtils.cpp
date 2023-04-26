#include "WNetWork/WNetWorkUtils.h"
#include <cassert>
#include <iostream>

namespace wlb::network {


int   GetError() { return errno; }
char *ErrorToString(int error) { return strerror(error); }

timerfd_t CreateNewTimerfd() {
    // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    // return ::timerfd_create(CLOCK_REALTIME, 0);
}

bool SetTimerTime(timerfd_t fd, SetTimeFlag flag, const struct itimerspec *next_time, struct itimerspec *prev_time) {
    if(::timerfd_settime(fd, (int)flag, next_time, prev_time) == 0) {
        return true;
    }
    return false;
}

// -1 for errno
base_socket_type MakeSocket(enum AF_FAMILY family, enum AF_PROTOL protol) {
    if(protol == AF_PROTOL::TCP) {
        return ::socket(static_cast<int>(family), SOCK_STREAM, IPPROTO_TCP);
    } else {
        return ::socket(static_cast<int>(family), SOCK_DGRAM, IPPROTO_UDP);
    }
}
base_socket_type MakeTcpV4Socket() { return MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP); }
base_socket_type MakeUdpV4Socket() { return MakeSocket(AF_FAMILY::INET, AF_PROTOL::UDP); }

bool IpAddrToString(in_addr addr_, std::string *buf) {
    try {
        int32_t _len = 30;
        char   *_buf = new char[_len];
        if(::inet_ntop((int)AF_FAMILY::INET, (void *)&addr_, _buf, _len) == nullptr) {
            buf->clear();
            return false;
        }

        buf->assign(_buf);
        delete[] _buf;
        return true;
    } catch(const std::exception &e) {
        std::cerr << "IpAddrToString fail, error : " << e.what() << std::endl;
        return false;
    }
}

bool IpAddrToString(in6_addr addr, std::string *buf) {
    try {
        int32_t _len = 50;
        char   *_buf = new char[_len];
        if(::inet_ntop((int)AF_FAMILY::INET6, (void *)&addr, _buf, _len) == nullptr) {
            buf->clear();
            return false;
        }

        buf->assign(_buf);
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
    std::cout << "IPStringToAddress " << ip_str << std::endl;
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
        std::cout << "MakeSockAddr_in6 HtoNS" << std::endl;
        return false;
    }

    if(!IPStringToAddress(ip_address, &h_addr)) {
        std::cout << "MakeSockAddr_in6 IPStringToAddress err" << std::endl;
        return false;
    }

    addr->sin6_port     = h_port;
    addr->sin6_addr     = h_addr;
    addr->sin6_flowinfo = 0;
    return true;
}

bool Bind(base_socket_type socket, const std::string &host, uint16_t port, bool isv4) {
    if(isv4) {
        sockaddr_in ei{0};

        if(!MakeSockAddr_in(host, port, &ei)) {
            return false;
        }

        int32_t ok = ::bind(socket, (struct sockaddr *)&(ei), sizeof(ei));
        if(ok == 0) {
            return true;
        }
    } else {
        sockaddr_in6 ei{0};

        if(!MakeSockAddr_in6(host, port, &ei)) {
            return false;
        }

        std::cout << "Bind " << host << " : " << port << " " << sizeof(ei) << std::endl;

        int32_t ok = ::bind(socket, (struct sockaddr *)&(ei), sizeof(ei));
        if(ok == 0) {
            return true;
        }
    }

    return false;
}

bool Bind(base_socket_type socket, const WEndPointInfo &serverInfo) {

    auto t = WEndPointInfo::Dump(serverInfo);
    std::cout << "Bind " << std::get<0>(t) << " : " << std::get<1>(t) << " " << serverInfo.GetSockSize() << std::endl;

    auto ok = ::bind(socket, serverInfo.GetAddr(), serverInfo.GetSockSize());
    if(ok == 0) {
        return true;
    }

    return false;
}


base_socket_type MakeBindedSocket(const WEndPointInfo &info) {
    base_socket_type listen_sock = 0;
    auto             fami        = info.GetFamily();

    listen_sock = MakeSocket(fami, AF_PROTOL::UDP);
    if(listen_sock == -1) {
        std::cout << "make listened socket : make socket failed" << strerror(errno) << std::endl;
        return listen_sock;
    }

    if(Bind(listen_sock, info)) {
        return listen_sock;
    }
    close(listen_sock);
    return -1;
}

base_socket_type MakeListenedSocket(const WEndPointInfo &info) {
    base_socket_type listen_sock = 0;
    auto             fami        = info.GetFamily();

    listen_sock = MakeSocket(fami, AF_PROTOL::TCP);
    if(listen_sock == -1) {
        std::cout << "make listened socket : make socket failed " << strerror(errno) << std::endl;
        return listen_sock;
    }

    SetSocketReuseAddr(listen_sock);
    SetSocketReusePort(listen_sock);

    if(Bind(listen_sock, info)) {
        if(::listen(listen_sock, 1024) == 0) {
            return listen_sock;
        }
    }

    close(listen_sock);
    return -1;
}

base_socket_type Accept(base_socket_type socket, WEndPointInfo *info) {
    socklen_t        len = 0;
    sockaddr_in6     temp;
    base_socket_type clientsock = ::accept(socket, (sockaddr *)&temp, &len);
    if(clientsock < 0) {
        return -1;
    }
    // clang-format off
    info->Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return clientsock;
}

base_socket_type Accept4(base_socket_type socket, WEndPointInfo *info, int flags) {
    socklen_t        len = 0;
    sockaddr_in6     temp;
    base_socket_type clientsock = ::accept4(socket, (sockaddr *)&temp, &len, flags);
    if(clientsock < 0) {
        return -1;
    }
    // clang-format off
    info->Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return clientsock;
}

bool ConnectToHost(base_socket_type socket, const std::string &host, uint16_t port, bool isv4) {
    if(isv4) {
        sockaddr_in addr{};
        if(!MakeSockAddr_in(host, port, &addr)) {
            return false;
        }
        if(::connect(socket, (sockaddr *)&addr, sizeof(addr)) == 0) {
            return true;
        }
    } else {
        sockaddr_in6 addr{};
        if(!MakeSockAddr_in6(host, port, &addr)) {
            return false;
        }
        if(::connect(socket, (sockaddr *)&addr, sizeof(addr)) == 0) {
            return true;
        }
    }
    return false;
}
bool ConnectToHost(base_socket_type socket, const WEndPointInfo &info) {
    const auto _size = info.GetFamily() == AF_FAMILY::INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    if(::connect(socket, info.GetAddr(), _size) == 0) {
        return true;
    }

    return false;
}

/***************************************************
 * UDP Utils
 ****************************************************/

int32_t RecvFrom(base_socket_type socket, uint8_t *buf, uint32_t buf_len, WEndPointInfo* info) { 
    socklen_t        len = 0;
    sockaddr_in6     temp;
    auto recv_len = ::recvfrom(socket, buf, buf_len, 0, (sockaddr *)&temp, &len); 
    if(recv_len < 0) {
        return -1;
    }

    // clang-format off
    info->Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return recv_len;
}

/***************************************************
 * Socket Utils
 ****************************************************/
bool SetSocketNoBlock(base_socket_type socket) {
    if(::fcntl(socket, F_SETFL, ::fcntl(socket, F_GETFL, 0) | O_NONBLOCK) == -1) {
        return false;
    }
    return true;
}

bool SetSocketReuseAddr(base_socket_type socket) {
    int          opt = 1;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == -1) {
        return false;
    }
    return true;
}
bool SetSocketReusePort(base_socket_type socket) {
    int          opt = 1;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == -1) {
        return false;
    }
    return true;
}
bool SetSocketKeepAlive(base_socket_type socket) {
    int          opt = 1;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == -1) {
        return false;
    }
    return true;
}

bool SetTcpSocketNoDelay(base_socket_type socket) {
    int opt_val = 1;
    if(::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt_val, static_cast<socklen_t>(sizeof opt_val)) == 0) {
        return true;
    }
    return false;
}

WEndPointInfo *WEndPointInfo::MakeWEndPointInfo(const std::string &address, uint16_t port, AF_FAMILY family) {
    auto ep = new WEndPointInfo();

    if(ep->Assign(address, port, family)) {
        return ep;
    }

    delete ep;
    return nullptr;
}

std::tuple<std::string, uint16_t> WEndPointInfo::Dump(const WEndPointInfo &info) {
    std::string s;
    uint16_t    p;
    bool        ok = false;

    if(info.family_ == AF_FAMILY::INET) {
        auto *SockAddrIn = reinterpret_cast<const struct sockaddr_in *>(&info.addr_);

        ok = IpAddrToString(SockAddrIn->sin_addr, &s);
        if(!ok) {
            goto err;
        }

        ok = NtoHS(SockAddrIn->sin_port, &p);
        if(!ok) {
            goto err;
        }
    } else {
        auto *sockAddrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(&info.addr_);

        ok = IpAddrToString(sockAddrIn6->sin6_addr, &s);
        if(!ok) {
            goto err;
        }
        ok = NtoHS(sockAddrIn6->sin6_port, &p);
        if(!ok) {
            goto err;
        }
    }

    return std::make_tuple(s, p);

err:
    return std::make_tuple<std::string, uint16_t>("error", 0);
}

bool WEndPointInfo::Assign(const std::string &address, uint16_t port, AF_FAMILY family) {
    this->family_ = family;
    bool ok       = false;

    switch(this->family_) {
    case AF_FAMILY::INET:
        ok = MakeSockAddr_in(address, port, (sockaddr_in *)&this->addr_);
        if(!ok) {
            std::cout << "MakeSockAddr_in falied" << std::endl;
            goto err;
        }
        break;
    case AF_FAMILY::INET6:
        ok = MakeSockAddr_in6(address, port, (sockaddr_in6 *)&this->addr_);
        if(!ok) {
            std::cout << "MakeSockAddr_in6 falied" << std::endl;
            goto err;
        }
        break;

    default:
        // never achieve
        std::cout << "never achieve" << std::endl;
        goto err;
        break;
    }

    this->SetHash();
    return true;
err:
    return false;
}

bool WEndPointInfo::Assign(const sockaddr *addr, AF_FAMILY family) {
    this->family_ = family;

    switch(this->family_) {
    case AF_INET:
        memcpy(this->addr_, addr, sizeof(sockaddr_in));
        break;
    case AF_INET6:
        memcpy(this->addr_, addr, sizeof(sockaddr_in6));
    default:
        assert("never acheive");
        break;
    }

    this->SetHash();
    return true;
}

WEndPointInfo *WEndPointInfo::Emplace(const sockaddr *addr, AF_FAMILY family) {
    auto info     = new WEndPointInfo();
    info->family_ = family;

    switch(info->family_) {
    case AF_INET:
        memcpy(info->addr_, addr, sizeof(sockaddr_in));
        break;
    case AF_INET6:
        memcpy(info->addr_, addr, sizeof(sockaddr_in6));
    default:
        assert("never acheive");
        break;
    }

    info->SetHash();
    return info;
}
} // namespace wlb::network
