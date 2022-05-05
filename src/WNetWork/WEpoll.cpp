#include "WNetWork/WEpoll.hpp"
#include <iostream>
#include <cassert>

#if defined(OS_IS_LINUX)

namespace wlb::NetWork {

epoll_type CreateNewEpollFd() {
    return epoll_create(1);
}

bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events) {
    struct epoll_event event{};
    event.data.fd = socket;
    event.events  = events;

    if (::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events) {
    struct epoll_event event{};
    event.data.fd = socket;
    event.events  = events;

    if (::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event{};
    event.data   = data;
    event.events = events;

    if (::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}
bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event{};
    event.data   = data;
    event.events = events;

    if (::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollRemoveSocket(epoll_type epoll, base_socket_type socket) {
    if (::epoll_ctl(epoll, EPOLL_CTL_DEL, socket, nullptr) == 0) {
        return true;
    }
    return false;
}

int32_t EpollGetEvents(epoll_type epoll, struct epoll_event *events, int32_t events_size, int32_t timeout) {
    if (events_size <= 0) {
        return 0;
    }

    return ::epoll_wait(epoll, events, events_size, timeout);
}

void CloseEpoll(epoll_type epoll) {
    ::close(epoll);
}

WBaseEpoll::~WBaseEpoll() {
    this->Close();
}

bool WBaseEpoll::Init() {
    this->epoll_fd_ = CreateNewEpollFd();
    if (this->epoll_fd_ == -1) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

void WBaseEpoll::Close() {
    if (this->epoll_fd_ != -1) {
        CloseEpoll(this->epoll_fd_);
        this->epoll_fd_ = -1;
    }
}

bool WBaseEpoll::AddSocket(base_socket_type socket, uint32_t events) {
    assert(socket != -1);
    assert(events > 0);

    if (!EpollAddSocket(this->epoll_fd_, socket, events)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events) {
    assert(socket != -1);

    if (!EpollModifySocket(this->epoll_fd_, socket, events)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}
bool WBaseEpoll::AddSocket(base_socket_type socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if (!EpollAddSocket(this->epoll_fd_, socket, events, data)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if (!EpollModifySocket(this->epoll_fd_, socket, events, data)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

void WBaseEpoll::RemoveSocket(base_socket_type socket) {
    assert(socket != -1);

    if (!EpollRemoveSocket(this->epoll_fd_, socket)) {
        this->errno_ = errno;
    }
}

int32_t WBaseEpoll::GetEvents(epoll_event *events, int32_t events_size, int32_t timeout) {
    auto res = EpollGetEvents(this->epoll_fd_, events, events_size, timeout);
    if (res == -1) {
        this->errno_ = errno;
        return -1;
    }
    return res;
}
int16_t WBaseEpoll::GetErrorNo() {
    int16_t e    = this->errno_;
    this->errno_ = -1;
    return e;
}



////////////////////////////////////////////////////////////////////////
// WEpoll

bool WEpoll::Init(uint32_t events_size) {
    if (!WBaseEpoll::Init()) {
        return false;
    }

    events_size < default_events_size_ ?
            this->events_size_ = default_events_size_ :
            this->events_size_ = events_size;

    return true;
}

void WEpoll::GetAndEmitEvents(int32_t timeout) {
    this->events_ = new(std::nothrow) epoll_event[this->events_size_];
    if (events_ == nullptr) {
        assert(events_);
        return;
    }

    int32_t curr_events_size;
    curr_events_size =
            WBaseEpoll::GetEvents(events_, static_cast<int32_t>(this->events_size_), timeout);

    if (curr_events_size == -1) {
        this->errno_ = errno;
        return;
    } else {
        for (int32_t index = 0; index < curr_events_size; ++index) {
            WHandlerData              *data     = (WHandlerData *) events_[index].data.ptr;
            //
            WNetWorkHandler::Listener *listener = data->listener;
            base_socket_type          sock      = data->socket;

            if (events_[index].events & EPOLLHUP) { // 对端已经关闭 受到最后一次挥手
                std::cout << "epoll-hup" << std::endl;
            }
            if (events_[index].events & EPOLLERR) {
                std::cout << "epoll-err" << std::endl;
            }
            if (events_[index].events & EPOLLRDHUP) {   // 对端关闭写，
                std::cout << "epoll-rdhup" << std::endl;
            }
            if (events_[index].events & EPOLLIN) {
                std::cout << "epoll-in" << std::endl;
            }
            if (events_[index].events & EPOLLOUT) {
                std::cout << "epoll-out" << std::endl;
            }

            if (events_[index].events & EPOLLHUP) {
                // 对端已经关闭 受到最后一次挥手
                listener->OnClosed();
                RemoveSocket(sock);
            } else if (events_[index].events & EPOLLERR) {
                listener->OnError(errno);
                RemoveSocket(sock);
            } else if (events_[index].events & EPOLLRDHUP) {
                // 对端关闭写
                // peer shutdown write
                listener->OnShutdown();
            } else if (events_[index].events & EPOLLIN) {
                listener->OnRead();
            } else if (events_[index].events & EPOLLOUT) {
                listener->OnWrite();
            }
        }

        this->events_size_ > curr_events_size ?
                this->events_size_ = 10 + (curr_events_size / 10 + this->events_size_ * 9 / 10) :
                this->events_size_ = curr_events_size * 3 / 2;

        assert(this->events_size_ > 0);
        // 
    }

    delete[] this->events_;
}

void WEpoll::Close() {
    if (this->events_ != nullptr) {
        delete[] this->events_;
        this->events_ = nullptr;
    }
    WBaseEpoll::Close();
}

bool WEpoll::AddSocket(WHandlerData *data, uint32_t op) {
    epoll_data_t _data;
    uint32_t     register_event;

    _data.ptr = data;

    // 翻译注册事件
    register_event = GetEpollEventsFromOP(op);
    return WBaseEpoll::AddSocket(data->socket, register_event, _data);
}

bool WEpoll::ModifySocket(WHandlerData *data, uint32_t op) {
    epoll_data_t _data;
    _data.ptr = data;

    uint32_t _event;
    _event = GetEpollEventsFromOP(op);
    return WBaseEpoll::ModifySocket(data->socket, _event, _data);
}

void WEpoll::RemoveSocket(base_socket_type socket) {
    WBaseEpoll::RemoveSocket(socket);
}

uint32_t WEpoll::GetEpollEventsFromOP(uint32_t op) {
    uint32_t _events = 0;
    if (op & OP_IN) {
        _events |= EPOLLIN;
    }
    if (op & OP_OUT) {
        _events |= EPOLLOUT;
    }
    if (op & OP_ERR) {
        _events |= EPOLLERR;
    }
    if (op & OP_SHUT) {
        _events |= EPOLLRDHUP;
    }
    if (op & OP_CLOS) {
        _events |= EPOLLHUP;
    }

    return _events;
}
int16_t WEpoll::GetErrorNo() {
    return WBaseEpoll::GetErrorNo();
}

/////////////////////////////////
// WTimerEpoll

bool WTimerEpoll::Init() {
    if (!WBaseEpoll::Init()) {
        return false;
    }

    return true;
}

void WTimerEpoll::Close() {
    if (this->events_ != nullptr) {
        delete[] this->events_;
        this->events_ = nullptr;
    }
    WBaseEpoll::Close();
}

void WTimerEpoll::GetAndEmitTimer(int32_t timeout) {
    this->events_ = new(std::nothrow) epoll_event[this->events_size_];
    if (events_ == nullptr) {
        assert(events_);
        return;
    }

    int32_t curr_events_size = WBaseEpoll::GetEvents(events_, this->events_size_, timeout);

    if (curr_events_size == -1) {
        // error
        std::cout << "error no:" << errno << " " << strerror(errno) << std::endl;
        this->errno_ = errno_;
    } else {
        Listener *listener;
        WTimer   *timer;
        timerfd  fd;

        for (int32_t index = 0; index < curr_events_size; ++index) {
            WTimerHandlerData *data = (WTimerHandlerData *) events_[index].data.ptr;
            //
            listener = data->listener;
            timer    = data->timer_;
            fd       = data->timer_fd_;

            if (events_[index].events & EPOLLIN) {
                listener->OnTime(timer);
                uint64_t exp = 0;
                read(fd, &exp, sizeof(uint64_t));
            }
        }

        this->events_size_ > curr_events_size ?
                this->events_size_ = 10 + (curr_events_size / 10 + this->events_size_ * 9 / 10) :
                this->events_size_ = curr_events_size * 3 / 2;

        // 
    }

    delete[] this->events_;
}

void WTimerEpoll::AddTimer(WTimerHandlerData *data) {
    epoll_data_t _data;
    _data.ptr = data;

    uint32_t event = 0;
    event |= EPOLLET;
    event |= EPOLLIN;

    WBaseEpoll::AddSocket(data->timer_fd_, event, _data);
}

void WTimerEpoll::RemoveTimer(WTimerHandlerData *data) {
    WBaseEpoll::RemoveSocket(data->timer_fd_);
}

int16_t WTimerEpoll::GetErrorNo() {
    return WBaseEpoll::GetErrorNo();
}

} // namespace wlb::NetWork

#endif // OS_IS_LINUX
