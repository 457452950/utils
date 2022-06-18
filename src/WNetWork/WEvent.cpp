#include "WNetWork/WEvent.h"

#include <cassert>
#include "WNetWork/WSelect.h"
#include "WNetWork/WEpoll.h"


namespace wlb::network {

WEventHandle *CreateNetHandle(HandleType type) {
    switch(type) {
    case HandleType::EPOLL:
        return new WEpoll;

    case HandleType::SELECT:
        return new WSelect;

    default:
        assert(type);
        break;
    }
}

} // namespace wlb::network
