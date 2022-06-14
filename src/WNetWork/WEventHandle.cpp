#include "WNetWork/WEventHandle.h"
#include <algorithm>


namespace wlb::network {

WEventHandle::fd_list_item WEventHandle::NewEvent(base_socket_type socket, event_data::user_data_ptr user_data) {
    if(!this->AddSocket(socket, user_data)) {
        return this->list.end();
    }

    this->list.emplace_front(socket, user_data);
    return this->list.begin();
}

void WEventHandle::EraseEvent(fd_list_item item) {
    this->DelSocket(item->GetSocket());
    this->list.erase_after(item);
}

bool WSelect::AddSocket(base_socket_type socket, event_data::user_data_ptr user_data) {
    if(this->fd_count_ >= 1024) {
        return false;
    }

    max_fd_ = std::max(socket, max_fd_);
    return true;
}

} // namespace wlb::network
