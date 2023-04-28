#include "WNetWork/WEpoll.h"
#include <cassert>
#include <iostream>

#include "WDebugger.hpp"

namespace wlb::network {

using namespace debug;

epoll_type CreateNewEpollFd() { return epoll_create(1); }

bool EpollAddSocket(epoll_type epoll, socket_t socket, uint32_t events) {
    struct epoll_event event {};
    event.data.fd = socket;
    event.events  = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollModifySocket(epoll_type epoll, socket_t socket, uint32_t events) {
    struct epoll_event event {};
    event.data.fd = socket;
    event.events  = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollAddSocket(epoll_type epoll, socket_t socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event {};
    event.data   = data;
    event.events = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}
bool EpollModifySocket(epoll_type epoll, socket_t socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event {};
    event.data   = data;
    event.events = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollRemoveSocket(epoll_type epoll, socket_t socket) {
    if(::epoll_ctl(epoll, EPOLL_CTL_DEL, socket, nullptr) == 0) {
        return true;
    }
    return false;
}

int32_t EpollGetEvents(epoll_type epoll, struct epoll_event *events, int32_t events_size, int32_t timeout) {
    assert(events_size > 0);

    return ::epoll_wait(epoll, events, events_size, timeout);
}

void CloseEpoll(epoll_type epoll) { ::close(epoll); }


/*************************************************
 * base epoll 
**************************************************/

WBaseEpoll::WBaseEpoll() { DEBUGADD("WBaseEpoll"); }
WBaseEpoll::~WBaseEpoll() {
    this->Close();
    DEBUGRM("WBaseEpoll");
}

bool WBaseEpoll::Init() {
    this->epoll_fd_ = CreateNewEpollFd();
    if(this->epoll_fd_ == -1) {
        return false;
    }
    return true;
}

void WBaseEpoll::Close() {
    if(this->epoll_fd_ != -1) {
        CloseEpoll(this->epoll_fd_);
        this->epoll_fd_ = -1;
    }
}

bool WBaseEpoll::AddSocket(socket_t socket, uint32_t events) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollAddSocket(this->epoll_fd_, socket, events)) {
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(socket_t socket, uint32_t events) {
    assert(socket != -1);

    if(!EpollModifySocket(this->epoll_fd_, socket, events)) {
        return false;
    }
    return true;
}

bool WBaseEpoll::AddSocket(socket_t socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollAddSocket(this->epoll_fd_, socket, events, data)) {
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(socket_t socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollModifySocket(this->epoll_fd_, socket, events, data)) {
        return false;
    }
    return true;
}

bool WBaseEpoll::RemoveSocket(socket_t socket) {
    assert(socket != -1);

    return EpollRemoveSocket(this->epoll_fd_, socket);
}

int32_t WBaseEpoll::GetEvents(epoll_event *events, int32_t events_size, int32_t timeout) {
    auto res = EpollGetEvents(this->epoll_fd_, events, events_size, timeout);
    // if(res == -1) {
    //     return -1;
    // }
    return res;
}






} // namespace wlb::network
