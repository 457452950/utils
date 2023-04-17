#include "WNetWork/WNetWorkUtils.h"
#include <iostream>

namespace wlb::network {

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

    HtoNS(port, &h_port);

    if(!IPStringToAddress(ip_address, &h_addr)) {
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

        int32_t ok = ::bind(socket, (struct sockaddr *)&(ei), sizeof(ei));
        if(ok == 0) {
            return true;
        }
    }

    return false;
}

bool Bind(base_socket_type socket, const WEndPointInfo &serverInfo) {

    const auto _size = serverInfo.family == AF_FAMILY::INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);

    int32_t ok = ::bind(socket, (struct sockaddr *)&(serverInfo.addr), _size);
    if(ok == 0) {
        return true;
    }


    return false;
}


base_socket_type MakeListenedSocket(const WEndPointInfo &info, enum AF_PROTOL protol) {
    base_socket_type listen_sock = 0;
    auto             fami        = info.family;

    listen_sock = MakeSocket(fami, protol);
    if(listen_sock == -1) {
        std::cout << "make listened socket : make socket failed" << strerror(errno) << std::endl;
        return listen_sock;
    }

    if(Bind(listen_sock, info)) {
        if(::listen(listen_sock, 1024) == 0) {
            return listen_sock;
        }
    }

    close(listen_sock);
    return -1;
}

base_socket_type Accept(base_socket_type socket, WEndPointInfo *info) {
    socklen_t len = 0;

    base_socket_type clientsock = ::accept(socket, (struct sockaddr *)&info->addr, &len);
    if(clientsock < 0) {
        return -1;
    }
    // clang-format off
    info->family = 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6;
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
    const auto _size = info.family == AF_FAMILY::INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    if(::connect(socket, (sockaddr *)&info.addr, _size) == 0) {
        return true;
    }

    return false;
}

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

WEndPointInfo *WEndPointInfo::MakeWEndPointInfo(const std::string &address, uint16_t port, bool isv4) {
    auto ep    = new WEndPointInfo();
    ep->family = isv4 ? AF_FAMILY::INET : AF_FAMILY::INET6;
    bool ok    = false;

    switch(ep->family) {
    case AF_FAMILY::INET:
        ok = MakeSockAddr_in(address, port, &ep->addr.addr4);
        if(!ok) {
            goto err;
        }
        break;
    case AF_FAMILY::INET6:
        ok = MakeSockAddr_in6(address, port, &ep->addr.addr6);
        if(!ok) {
            goto err;
        }
        break;

    default:
        // never achieve
        goto err;
        break;
    }

    return ep;
err:
    delete ep;
    return nullptr;
}

std::tuple<std::string, uint16_t> WEndPointInfo::Dump(const WEndPointInfo &info) {
    std::string s;
    uint16_t    p;
    bool        ok = false;

    if(info.family == AF_FAMILY::INET) {
        ok = IpAddrToString(info.addr.addr4.sin_addr, &s);
        if(!ok) {
            goto err;
        }

        ok = NtoHS(info.addr.addr4.sin_port, &p);
        if(!ok) {
            goto err;
        }
    } else {
        ok = IpAddrToString(info.addr.addr6.sin6_addr, &s);
        if(!ok) {
            goto err;
        }
        ok = NtoHS(info.addr.addr6.sin6_port, &p);
        if(!ok) {
            goto err;
        }
    }

    return std::make_tuple(s, p);

err:
    return std::make_tuple<std::string, uint16_t>("", 0);
}


} // namespace wlb::network
