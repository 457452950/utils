#pragma once
#ifndef UTILS_WEVENT_H
#define UTILS_WEVENT_H

#include <cstdint>
#include <list>

#include "WNetWorkDef.h"
#include "WNetWorkUtils.h"


namespace wlb::network {

namespace KernelEventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace KernelEventType

template <typename UserData>
class WEventHandle {
public:
    virtual ~WEventHandle() {}

    using user_data_type = UserData;
    using user_data_ptr  = user_data_type *;

    struct hdle_option_t {
        base_socket_type socket_;    // native socket
        user_data_ptr    user_data_; // user data, void*
        uint8_t          events_;    // KernelEventType
    };

    using option_type      = hdle_option_t;
    using option_list      = std::list<hdle_option_t *>;
    using option_list_item = typename option_list::iterator;

    // call back
    using callback_type = void (*)(base_socket_type sock, user_data_ptr data);

    callback_type read_{nullptr};
    callback_type write_{nullptr};

    // control
    virtual option_list_item NewSocket(option_type *option)      = 0;
    virtual void             ModifySocket(option_list_item item) = 0;
    virtual void             DelSocket(option_list_item item)    = 0;

    // thread control
    virtual void Start()  = 0;
    virtual void Detach() = 0;
    virtual void Stop()   = 0;
    virtual void Join()   = 0;
};

class WBaseChannel;
using event_handle_t = WEventHandle<WBaseChannel>;
using event_handle_p = event_handle_t *;


} // namespace wlb::network


#endif // UTILS_WEVENT_H