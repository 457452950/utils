#pragma once 
#ifndef UTILS_WNETWORK_DEF_H
#define UTILS_WNETWORK_DEF_H

#include "../WOS.h"
#include <cstdint>

namespace wlb::network {

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
    const auto default_handle_type = HandleType::EPOLL;
#elif defined SERVER_USE_SELECT
    const auto default_handle_type = HandleType::SELECT;
#endif


struct EventContext;
using event_context_t = EventContext;
using event_context_p = event_context_t *;

} // namespace wlb::network



#endif // UTILS_WNETWORK_DEF_H



