#include "WNetWork/WEvent.h"

#include <cassert>
#include "WNetWork/WChannel.h"
#include "WNetWork/WEpoll.h"
#include "WNetWork/WSelect.h"


namespace wlb::network {

WEventHandle<WBaseChannel> *CreateNetHandle() { return new server_handle_type; }

} // namespace wlb::network
