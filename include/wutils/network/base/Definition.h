#pragma once
#ifndef UTIL_DEFINITION_H
#define UTIL_DEFINITION_H

#include <cstdint>

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

/**
 * listen 最大等待队列长度
 */
#define MAX_LISTEN_BACK_LOG 2048


}


#endif // UTIL_DEFINITION_H
