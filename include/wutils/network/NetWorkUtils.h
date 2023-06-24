// #pragma once
// #ifndef UTILS_NETWORK_UTILS_H
// #define UTILS_NETWORK_UTILS_H
//
// #include <cerrno>
// #include <cstdint>
// #include <cstring> // strerror
// #include <exception>
// #include <fcntl.h> // fcntl
// #include <memory>
// #include <string>
// #include <tuple>
// #include <unistd.h>
// #include <unordered_map> //std::hash
//
// #include <sys/socket.h>
// #include <sys/timerfd.h>
//
// #include <arpa/inet.h>   // inet_ntop inet_pton
// #include <netinet/in.h>
// #include <netinet/tcp.h> // tcp_nodelay
//
// #include "wutils/OS.h"
//
//
// namespace wutils::network {
//
// enum AF_FAMILY { INET = AF_INET, INET6 = AF_INET6 };
// enum AF_PROTOL { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };
//
// struct ENDPOINTINFO;
// using native_socket_t = int;
// using timerfd_t       = int;
//
//// 设置时间差的意义
// enum class SetTimeFlag {
//     REL = 0, // 相对时间
//     ABS = 1, // 绝对时间
// };
//
// timerfd_t CreateNewTimerFd();
// bool      SetTimerTime(timerfd_t                fd,
//                        SetTimeFlag              flag,
//                        const struct itimerspec *next_time,
//                        struct itimerspec       *prev_time = nullptr);
//
//
//// sockaddr
//// sockaddr_in      sockaddr_in6
//// in_addr          in6_addr
//
// native_socket_t MakeSocket(enum AF_FAMILY family, enum AF_PROTOL protol);
// native_socket_t MakeTcpV4Socket();
// native_socket_t MakeUdpV4Socket();
//
//
///***************************************************
// * IP Port Utils
// ****************************************************/
//
// bool IpAddrToString(in_addr addr, std::string &buf);
// bool IpAddrToString(in6_addr addr, std::string &buf);
//
// bool IPStringToAddress(const std::string &ip_str, in_addr *addr);
// bool IPStringToAddress(const std::string &ip_str, in6_addr *addr);
//
// bool HtoNS(uint16_t host_num, uint16_t *net_num);
// bool NtoHS(uint16_t net_num, uint16_t *host_num);
//
// bool MakeSockAddr_in(const std::string &ip_address, uint16_t port, sockaddr_in *addr);
// bool MakeSockAddr_in6(const std::string &ip_address, uint16_t port, sockaddr_in6 *addr);
//
//// server methods
// bool Bind(native_socket_t socket, const ENDPOINTINFO &serverInfo);
//
// native_socket_t MakeBindedSocket(const ENDPOINTINFO &info, bool reuse);
// native_socket_t MakeListenedSocket(const ENDPOINTINFO &info, bool reuse);
//
///***************************************************
// * TCP Utils
// ****************************************************/
//// return -1 if fail
// native_socket_t Accept(native_socket_t socket, ENDPOINTINFO &info);
// native_socket_t Accept4(native_socket_t socket, ENDPOINTINFO &info, int flags);
//
//// tcp is default and return -1 if fail
// native_socket_t ConnectToHost(const ENDPOINTINFO &info, AF_PROTOL = AF_PROTOL::TCP);
// bool            ConnectToHost(native_socket_t socket, const ENDPOINTINFO &info);
//
///***************************************************
// * UDP Utils
// ****************************************************/
//
// int32_t RecvFrom(native_socket_t socket, uint8_t *buf, uint32_t buf_len, ENDPOINTINFO &info);
//
///***************************************************
// * ISocket Utils
// ****************************************************/
//
//// socket function
// bool SetSocketNoBlock(native_socket_t socket);
// bool SetSocketReuseAddr(native_socket_t socket);
// bool SetSocketReusePort(native_socket_t socket);
// bool SetSocketKeepAlive(native_socket_t socket);
//
//// tcp socket function
// bool SetTcpSocketNoDelay(native_socket_t socket);
//
//// IP + port + family
// struct ENDPOINTINFO {
//
//     static ENDPOINTINFO *MakeWEndPointInfo(const std::string &address, uint16_t port, AF_FAMILY family);
//
//     bool Assign(const std::string &address, uint16_t port, AF_FAMILY family);
//     bool Assign(const sockaddr *sock, AF_FAMILY family);
//
//     static ENDPOINTINFO *Emplace(const sockaddr *addr, AF_FAMILY family);
//
//     const sockaddr *GetAddr() const { return (sockaddr *)&addr_; }
//     AF_FAMILY       GetFamily() const { return family_; }
//     unsigned long   GetSockSize() const {
//         return family_ == AF_FAMILY::INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
//     }
//
//     static std::tuple<std::string, uint16_t> Dump(const ENDPOINTINFO &);
//
// private:
//     AF_FAMILY family_{AF_FAMILY::INET};
//     uint8_t   addr_[sizeof(sockaddr_in6)]{0};
//
// private:
//     /*
//              * Hash for IPv4
//              *
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//              |              PORT             |             IP                |
//              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//              |              IP               |                           |F|P|
//              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//              *
//              * Hash for IPv6
//              *
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//              |              PORT             | IP[0] ^  IP[1] ^ IP[2] ^ IP[3]|
//              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//              |IP[0] ^  IP[1] ^ IP[2] ^ IP[3] |          IP[0] >> 16      |F|P|
//              +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//              */
//     void SetHash() {
//
//         switch(this->family_) {
//         case AF_INET: {
//             auto *SockAddrIn = reinterpret_cast<const struct sockaddr_in *>(&this->addr_);
//
//             const uint64_t address = ntohl(SockAddrIn->sin_addr.s_addr);
//             const uint64_t port    = (ntohs(SockAddrIn->sin_port));
//
//             this->hash = port << 48;
//             this->hash |= address << 16;
//             this->hash |= 0x0000; // AF_INET.
//
//             break;
//         }
//
//         case AF_INET6: {
//             auto *sockAddrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(&this->addr_);
//             auto *a           = reinterpret_cast<const uint32_t *>(std::addressof(sockAddrIn6->sin6_addr));
//
//             const auto     address1 = a[0] ^ a[1] ^ a[2] ^ a[3];
//             const auto     address2 = a[0];
//             const uint64_t port     = ntohs(sockAddrIn6->sin6_port);
//
//             this->hash = port << 48;
//             this->hash |= static_cast<uint64_t>(address1) << 16;
//             this->hash |= address2 >> 16 & 0xFFFC;
//             this->hash |= 0x0002; // AF_INET6.
//
//             break;
//         }
//         }
//
//         // note:no safed protocol
//         // Override least significant bit with protocol information:
//         // - If UDPPointer, start with 0.
//         // - If TCP, start with 1.
//         // if (this->protocol == AF_PROTOL::UDPPointer)
//         // {
//         // 	this->hash |= 0x0000;
//         // }
//         // else
//         // {
//         // 	this->hash |= 0x0001;
//         // }
//     }
//
// public:
//     uint64_t hash{0u};
//     bool     operator==(const ENDPOINTINFO &other) const noexcept { return this->hash == other.hash; }
//     bool     operator!=(const ENDPOINTINFO &other) const noexcept { return this->hash != other.hash; }
// };
//
//
// } // namespace wutils::network
//
// namespace std {
// template <>
// class hash<wutils::network::ENDPOINTINFO> {
// public:
//     size_t operator()(const wutils::network::ENDPOINTINFO &it) const noexcept { return it.hash; }
// };
// } // namespace std
//
// #endif // UTILS_NETWORK_UTILS_H