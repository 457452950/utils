#include "wutils/network//Epoll.h"
#include <cassert>
#include <iostream>

#include "wutils/Debugger.hpp"

namespace wutils::network::epoll {


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

BaseEpoll::BaseEpoll() {}
BaseEpoll::~BaseEpoll() { this->Close(); }

bool BaseEpoll::Init() {
    this->epoll_fd_ = CreateNewEpollFd();
    if(this->epoll_fd_ == -1) {
        return false;
    }
    return true;
}

void BaseEpoll::Close() {
    if(this->epoll_fd_ != -1) {
        CloseEpoll(this->epoll_fd_);
        this->epoll_fd_ = -1;
    }
}

bool BaseEpoll::AddSocket(socket_t socket, uint32_t events) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollAddSocket(this->epoll_fd_, socket, events)) {
        return false;
    }
    return true;
}

bool BaseEpoll::ModifySocket(socket_t socket, uint32_t events) {
    assert(socket != -1);

    if(!EpollModifySocket(this->epoll_fd_, socket, events)) {
        return false;
    }
    return true;
}

bool BaseEpoll::AddSocket(socket_t socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollAddSocket(this->epoll_fd_, socket, events, data)) {
        return false;
    }
    return true;
}

bool BaseEpoll::ModifySocket(socket_t socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollModifySocket(this->epoll_fd_, socket, events, data)) {
        return false;
    }
    return true;
}

bool BaseEpoll::RemoveSocket(socket_t socket) {
    assert(socket != -1);

    return EpollRemoveSocket(this->epoll_fd_, socket);
}

int32_t BaseEpoll::GetEvents(epoll_event *events, int32_t events_size, int32_t timeout) {
    auto res = EpollGetEvents(this->epoll_fd_, events, events_size, timeout);
    return res;
}


} // namespace wutils::network::epoll
