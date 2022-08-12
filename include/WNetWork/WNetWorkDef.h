#pragma once 
#ifndef WNETWORKDEF_H
#define WNETWORKDEF_H

#include "../WOS.h"

namespace wlb::network {

#ifdef OS_IS_LINUX
  #define SERVER_USE_EPOLL
#else
  #define SERVER_USE_SELECT
#endif

template<typename UserData>
class WEventHandle;
template <typename UserData>
class WSelect;
template <typename UserData>
class WEpoll;

class WBaseChannel {
public:
    virtual ~WBaseChannel() {}

    virtual void ChannelIn()  = 0;
    virtual void ChannelOut() = 0;
};


#ifdef SERVER_USE_EPOLL
  using server_handle_type = WEpoll<WBaseChannel>;
#elif defined SERVER_USE_SELECT
  using server_handle_type = WSelect<WBaseChannel>;
#endif

using server_handle_ptr  = server_handle_type *;

WEventHandle<WBaseChannel> *CreateNetHandle();


struct EventContext;
using event_context_t = EventContext;
using event_context_p = event_context_t *;

} // namespace wlb::network



#endif



