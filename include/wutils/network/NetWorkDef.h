#pragma once
#ifndef UTILS_NETWORK_DEF_H
#define UTILS_NETWORK_DEF_H

#include "wutils/OS.h"
#include <cstdint>

namespace wutils::network {

#ifdef OS_IS_LINUX
#define SERVER_USE_EPOLL
#else
#define SERVER_USE_SELECT
#endif


enum class HandleType : uint8_t {
    SELECT,
    EPOLL,
};

#ifdef SERVER_USE_EPOLL
const auto    default_handle_type = HandleType::EPOLL;
#elif defined SERVER_USE_SELECT
const auto default_handle_type = HandleType::SELECT;
#endif


#define MAX_CHANNEL_SEND_SIZE 100 * 1024        // 10k
#define MAX_CHANNEL_RECV_BUFFER_SIZE 100 * 1024 // 8k
#define MAX_CHANNEL_SEND_BUFFER_SIZE 100 * 1024 // 8k

#define MAX_LAN_UDP_PACKAGE_LEN 1472
#define MAX_WAN_UDP_PACKAGE_LEN 548
#define MAX_UDP_BUFFER_LEN 1500


struct EventFactory;
using event_context_t = EventFactory;
using event_context_p = event_context_t *;

} // namespace wutils::network


#endif // UTILS_NETWORK_DEF_H
