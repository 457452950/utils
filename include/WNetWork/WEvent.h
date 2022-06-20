#pragma once

#include <cstdint>
#include <list>

#include "WNetWorkUtils.h"


namespace wlb::network {

namespace KernelEventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace KernelEventType

template <typename U>
class WEventHandle {
public:
    virtual ~WEventHandle() {}

    using user_data_type = U;
    using user_data_ptr  = user_data_type *;
    struct hdle_data_t {
        base_socket_type socket_;
        user_data_ptr    user_data_;
        uint8_t          events_;
    };
    using fd_list      = std::list<hdle_data_t *>;
    using fd_list_item = typename fd_list::iterator;

    // call back
    typedef void (*callback_type)(base_socket_type sock, user_data_ptr data);

    callback_type read_{nullptr};
    callback_type write_{nullptr};

    // control
    virtual fd_list_item NewSocket(base_socket_type socket, uint8_t events, user_data_ptr user_data = nullptr) = 0;
    virtual void         ModifySocket(fd_list_item item)                                                       = 0;
    virtual void         DelSocket(fd_list_item item)                                                          = 0;

    // thread control
    virtual void Start()  = 0;
    virtual void Detach() = 0;
    virtual void Stop()   = 0;
    virtual void Join()   = 0;
};
;


template <typename U>
class WSelect;
template <typename U>
class WEpoll;

class WBaseChannel {
public:
    virtual ~WBaseChannel() {}

    virtual void ChannelIn()  = 0;
    virtual void ChannelOut() = 0;
};

#ifdef OS_IS_LINUX
#define SERVER_USE_EPOLL
#else
#define SERVER_USE_SELECT
#endif

#ifdef SERVER_USE_EPOLL
using server_handle_type = WEpoll<WBaseChannel>;
#elif defined SERVER_USE_SELECT
using server_handle_type = WSelect<WBaseChannel>;
#endif

using server_handle_ptr  = server_handle_type *;

WEventHandle<WBaseChannel> *CreateNetHandle();

class ReadChannel;
class WriteChannel;

struct EventContext;
using event_context_t = EventContext;
using event_context_p = event_context_t *;

} // namespace wlb::network
