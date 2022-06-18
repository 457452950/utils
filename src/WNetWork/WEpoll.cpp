#include "WNetWork/WEpoll.h"
#include <cassert>
#include <iostream>


namespace wlb::network {

epoll_type CreateNewEpollFd() { return epoll_create(1); }

bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events) {
    struct epoll_event event {};
    event.data.fd = socket;
    event.events  = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events) {
    struct epoll_event event {};
    event.data.fd = socket;
    event.events  = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event {};
    event.data   = data;
    event.events = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}
bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event {};
    event.data   = data;
    event.events = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

bool EpollRemoveSocket(epoll_type epoll, base_socket_type socket) {
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

WBaseEpoll::~WBaseEpoll() { this->Close(); }

bool WBaseEpoll::Init() {
    this->epoll_fd_ = CreateNewEpollFd();
    if(this->epoll_fd_ == -1) {
        this->errno_ = errno;
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

bool WBaseEpoll::AddSocket(base_socket_type socket, uint32_t events) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollAddSocket(this->epoll_fd_, socket, events)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events) {
    assert(socket != -1);

    if(!EpollModifySocket(this->epoll_fd_, socket, events)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

bool WBaseEpoll::AddSocket(base_socket_type socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    // std::cout << "WBaseEpoll::AddSocket in & " << data.ptr << std::endl;
    // std::cout << "WBaseEpoll::AddSocket socket : " << socket << std::endl;

    if(!EpollAddSocket(this->epoll_fd_, socket, events, data)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events, epoll_data_t data) {
    assert(socket != -1);
    assert(events > 0);

    if(!EpollModifySocket(this->epoll_fd_, socket, events, data)) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

void WBaseEpoll::RemoveSocket(base_socket_type socket) {
    assert(socket != -1);

    if(!EpollRemoveSocket(this->epoll_fd_, socket)) {
        this->errno_ = errno;
    }
}

int32_t WBaseEpoll::GetEvents(epoll_event *events, int32_t events_size, int32_t timeout) {
    auto res = EpollGetEvents(this->epoll_fd_, events, events_size, timeout);
    if(res == -1) {
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

WEpoll::WEpoll() { assert(ep.Init()); };
WEpoll::~WEpoll() {
    while(!this->list.empty()) {
        delete this->list.front();
        this->list.pop_front();
    }
};

WEpoll::fd_list_item WEpoll::NewSocket(base_socket_type socket, uint8_t events, user_data_ptr user_data) {
    hdle_data_t *ed = new hdle_data_t;
    uint32_t     ev = 0;
    epoll_data_t d{0};

    ed->events_    = events;
    ed->socket_    = socket;
    ed->user_data_ = user_data;

    if(events & KernelEventType::EV_IN) {
        ev |= EPOLLIN;
    }
    if(events & KernelEventType::EV_OUT) {
        ev |= EPOLLOUT;
    }

    d.ptr = ed;

    // // std::cout << "in evetns " << (int)ed->events_ << " - " << (int)events << " =" << std::endl;
    // // std::cout << "in & " << ed << std::endl;
    // std::cout << "WEpoll::NewSocket new hdle_data_t & " << d.ptr << std::endl;

    ep.AddSocket(socket, ev, d);

    ++this->fd_count_;

    this->list.push_front(ed);
    return this->list.begin();
}

void WEpoll::ModifySocket(fd_list_item item) {
    uint32_t     ev     = 0;
    uint8_t      events = (*item)->events_;
    epoll_data_t d{0};

    if(events & KernelEventType::EV_IN) {
        ev |= EPOLLIN;
    }
    if(events & KernelEventType::EV_OUT) {
        ev |= EPOLLOUT;
    }

    d.ptr = (*item);

    ep.ModifySocket((*item)->socket_, ev, d);
}

void WEpoll::DelSocket(WEpoll::fd_list_item item) {
    --this->fd_count_;
    this->ep.RemoveSocket((*item)->socket_);
    this->list.erase(item);
    delete *item;
}

void WEpoll::EventLoop() {
    epoll_event *events;

    // std::cout << "wepoll event loop start" << std::endl;

    while(this->active_) {
        int events_size = fd_count_;
        // std::cout << "array len " << events_size << std::endl;
        events = new epoll_event[events_size];

        events_size = ep.GetEvents(events, events_size, -1);
        // std::cout << "get events return " << events_size << std::endl;

        if(events_size == -1) {
            // std::cout << "error : " << strerror(errno) << std::endl;
            break;
        } else if(events_size == 0) {
            continue;
        }

        // for(auto i : this->list) {
        //     // std::cout << i->socket_ << std::endl;
        // }

        for(size_t i = 0; i < events_size; ++i) {
            // std::cout << "events index " << i << std::endl;

            uint32_t     ev   = events[i].events;
            hdle_data_t *data = (hdle_data_t *)events[i].data.ptr;
            assert(data);
            base_socket_type sock = data->socket_;
            uint8_t          eev  = data->events_;

            // // std::cout << "out & " << data << std::endl;
            // // std::cout << ev << " - - " << (int)eev << std::endl;

            // std::cout << "socket " << sock << std::endl;
            assert(sock > 0);

            if(ev & EPOLLIN && eev & KernelEventType::EV_IN) {
                if(this->read_) {
                    this->read_(sock, data->user_data_);
                    // std::cout << "read over" << std::endl;
                }
            }
            if(ev & EPOLLOUT && eev & KernelEventType::EV_OUT) {
                if(this->write_) {
                    this->write_(sock, data->user_data_);
                }
            }
        }

        // std::cout << "delete[] events" << std::endl;
        delete[] events;
    }
}

void WEpoll::Start() {
    this->active_      = true;
    this->work_thread_ = new std::thread(&WEpoll::EventLoop, this);
}

void WEpoll::Detach() { this->work_thread_->detach(); }

void WEpoll::Stop() { this->active_ = false; }

void WEpoll::Join() {
    if(this->work_thread_ && this->work_thread_->joinable()) {
        this->work_thread_->join();
    }
}

} // namespace wlb::network
