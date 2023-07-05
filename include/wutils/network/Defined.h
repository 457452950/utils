#pragma once
#ifndef UTILS_NETWORK_DEF_H
#define UTILS_NETWORK_DEF_H

#include <cstdint>
#include <netinet/in.h> // AF_INET IPPROTO_TCP

namespace wutils::network {

enum class ContextType : uint8_t {
    SELECT,
    EPOLL,
};

#define CONTEXT_USE_EPOLL
// #define CONTEXT_USE_SELECT

#ifdef CONTEXT_USE_EPOLL
const auto    default_context_type = ContextType::EPOLL;
#elif defined CONTEXT_USE_SELECT
const auto default_context_type = ContextType::SELECT;
#endif


#define MAX_CHANNEL_SEND_SIZE (16 * 1024ULL)         // 16k
#define MAX_CHANNEL_RECV_BUFFER_SIZE (320 * 1024ULL) // 320k
#define MAX_CHANNEL_SEND_BUFFER_SIZE (160 * 1024ULL) // 160k

#define MAX_LAN_UDP_PACKAGE_LEN 1472
#define MAX_WAN_UDP_PACKAGE_LEN 548
#define MAX_UDP_BUFFER_LEN 1500

enum AF_FAMILY { INET = AF_INET, INET6 = AF_INET6, UNIX = AF_UNIX };
enum AF_PROTOL { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

using socket_t = int32_t;
using socket_p = socket_t *;

constexpr socket_t INVALID_SOCKET = -1;

using timerfd_t = int32_t;
using timerfd_p = timerfd_t *;

// listen 最大等待队列长度
#define MAX_LISTEN_BACK_LOG 2048


} // namespace wutils::network


#endif // UTILS_NETWORK_DEF_H
