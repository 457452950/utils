#include "wutils/network/NetWorkUtils.h"

#include <cassert>
#include <iostream>

namespace wutils::network {


int         GetError() { return errno; }
const char *ErrorToString(int error) { return strerror(error); }

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
        std::cout << "MakeSockAddr_in6 HtoNS " << std::endl;
        return false;
    }

    if(!IPStringToAddress(ip_address, &h_addr)) {
        std::cout << "MakeSockAddr_in6 IPStringToAddress err " << std::endl;
        return false;
    }

    addr->sin6_port     = h_port;
    addr->sin6_addr     = h_addr;
    addr->sin6_flowinfo = 0;
    return true;
}

bool Bind(socket_t socket, const EndPointInfo &serverInfo) {

    auto t = EndPointInfo::Dump(serverInfo);
    std::cout << socket << " Bind " << std::get<0>(t) << " : " << std::get<1>(t) << " " << serverInfo.GetSockSize()
              << std::endl;

    auto ok = ::bind(socket, serverInfo.GetAddr(), serverInfo.GetSockSize());
    if(ok == 0) {
        return true;
    }

    return false;
}


socket_t MakeBindedSocket(const EndPointInfo &info, bool reuse) {
    socket_t bind_sock = -1;
    auto     fami      = info.GetFamily();

    auto [ip, port] = EndPointInfo::Dump(info);

    bind_sock = MakeSocket(fami, AF_PROTOL::UDP);
    if(bind_sock == -1) {
        std::cout << "MakeBindedSocket : make [" << ip << ":" << port << "] socket failed " << strerror(errno)
                  << std::endl;
        return -1;
    }

    if(reuse) {
        SetSocketReuseAddr(bind_sock);
        SetSocketReusePort(bind_sock);
    }

    if(Bind(bind_sock, info)) {
        return bind_sock;
    }

    std::cout << "MakeBindedSocket : " << bind_sock << " Bind [" << ip << ":" << port << "] failed " << strerror(errno)
              << std::endl;
    ::close(bind_sock);
    return -1;
}

socket_t MakeListenedSocket(const EndPointInfo &info, bool reuse) {
    socket_t listen_sock(-1);
    auto     fami = info.GetFamily();

    listen_sock = MakeSocket(fami, AF_PROTOL::TCP);
    if(listen_sock == -1) {
        std::cout << "make listened socket : make socket failed " << strerror(errno) << std::endl;
        return -1;
    }

    if(reuse) {
        SetSocketReuseAddr(listen_sock);
        SetSocketReusePort(listen_sock);
    }

    if(Bind(listen_sock, info)) {
        if(::listen(listen_sock, 1024) == 0) {
            return listen_sock;
        }
    }

    ::close(listen_sock);
    return -1;
}

socket_t Accept(socket_t socket, EndPointInfo &info) {
    socklen_t    len = 0;
    sockaddr_in6 temp;
    socket_t     clientsock = ::accept(socket, (sockaddr *)&temp, &len);
    if(clientsock < 0) {
        return -1;
    }
    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return clientsock;
}

socket_t Accept4(socket_t socket, EndPointInfo &info, int flags) {
    socklen_t    len = 0;
    sockaddr_in6 temp;
    socket_t     clientsock = ::accept4(socket, (sockaddr *)&temp, &len, flags);
    if(clientsock < 0) {
        return -1;
    }
    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return clientsock;
}


socket_t ConnectToHost(const EndPointInfo &info, AF_PROTOL protol) {
    auto lis_sock = MakeSocket(info.GetFamily(), protol);
    if(lis_sock == -1) {
        return -1;
    }
    return ConnectToHost(lis_sock, info);
}

bool ConnectToHost(socket_t socket, const EndPointInfo &info) {
    if(::connect(socket, info.GetAddr(), info.GetSockSize()) == 0) {
        return true;
    }

    return false;
}

/***************************************************
 * UDPPointer Utils
 ****************************************************/

int32_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, EndPointInfo &info) {
    socklen_t    len = 0;
    sockaddr_in6 temp{0};
    auto         recv_len = ::recvfrom(socket, buf, buf_len, 0, (sockaddr *)&temp, &len);
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
bool SetSocketNoBlock(socket_t socket) {
    if(::fcntl(socket, F_SETFL, ::fcntl(socket, F_GETFL, 0) | O_NONBLOCK) == -1) {
        return false;
    }
    return true;
}

bool SetSocketReuseAddr(socket_t socket) {
    int          opt = 1;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == -1) {
        return false;
    }
    return true;
}
bool SetSocketReusePort(socket_t socket) {
    int          opt = 1;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == -1) {
        return false;
    }
    return true;
}
bool SetSocketKeepAlive(socket_t socket) {
    int          opt = 1;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == -1) {
        return false;
    }
    return true;
}

bool SetTcpSocketNoDelay(socket_t socket) {
    int opt_val = 1;
    if(::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt_val, static_cast<socklen_t>(sizeof opt_val)) == 0) {
        return true;
    }
    return false;
}

EndPointInfo *EndPointInfo::MakeWEndPointInfo(const std::string &address, uint16_t port, AF_FAMILY family) {
    auto ep = new EndPointInfo();

    if(ep->Assign(address, port, family)) {
        return ep;
    }

    delete ep;
    return nullptr;
}

std::tuple<std::string, uint16_t> EndPointInfo::Dump(const EndPointInfo &info) {
    std::string s;
    uint16_t    p;
    bool        ok = false;

    if(info.family_ == AF_FAMILY::INET) {
        auto *SockAddrIn = reinterpret_cast<const struct sockaddr_in *>(&info.addr_);

        ok = IpAddrToString(SockAddrIn->sin_addr, s);
        if(!ok) {
            goto err;
        }

        ok = NtoHS(SockAddrIn->sin_port, &p);
        if(!ok) {
            goto err;
        }
    } else {
        auto *sockAddrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(&info.addr_);

        ok = IpAddrToString(sockAddrIn6->sin6_addr, s);
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

bool EndPointInfo::Assign(const std::string &address, uint16_t port, AF_FAMILY family) {
    this->family_ = family;
    bool ok       = false;

    switch(this->family_) {
    case AF_FAMILY::INET:
        // std::cout << "MakeSockAddr_in " << address << ":" << port << " family " << family << std::endl;
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

bool EndPointInfo::Assign(const sockaddr *addr, AF_FAMILY family) {
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

EndPointInfo *EndPointInfo::Emplace(const sockaddr *addr, AF_FAMILY family) {
    auto info     = new EndPointInfo();
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
} // namespace wutils::network
