#pragma once

#include <cstdint>
#include <list>

#include "WNetWorkUtils.h"

namespace wlb::network {

namespace KernelEventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace KernelEventType

class WEventHandle {
public:
    virtual ~WEventHandle() {}

    using user_data_ptr = void *;
    struct hdle_data_t {
        base_socket_type socket_;
        user_data_ptr    user_data_;
        uint8_t          events_;
    };
    using fd_list      = std::list<hdle_data_t *>;
    using fd_list_item = fd_list::iterator;

    // control
    virtual fd_list_item NewSocket(base_socket_type socket, uint8_t events, user_data_ptr user_data = nullptr) = 0;
    virtual void         ModifySocket(fd_list_item item)                                                       = 0;
    virtual void         DelSocket(fd_list_item item)                                                          = 0;

    // thread control
    virtual void Start()  = 0;
    virtual void Detach() = 0;
    virtual void Stop()   = 0;
    virtual void Join()   = 0;

    // call back
    typedef void (*callback_type)(base_socket_type sock, user_data_ptr data);

    callback_type read_{nullptr};
    callback_type write_{nullptr};
};

enum class HandleType {
#ifdef OS_IS_LINUX
    SELECT = 1 << 0,
    EPOLL  = 1 << 1,
#endif
};

class WSelect;
class WEpoll;
WEventHandle *CreateNetHandle(HandleType type);

class ReadChannel;
class WriteChannel;

struct EventContext;
using event_context_t = EventContext;
using event_context_p = event_context_t *;

} // namespace wlb::network
