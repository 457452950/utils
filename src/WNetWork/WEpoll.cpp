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
    ep_data_t   *ed = new ep_data_t;
    uint32_t     ev = 0;
    epoll_data_t d{0};

    ed->events_    = events;
    ed->socket_    = socket;
    ed->user_data_ = user_data;

    if(events & EventType::EV_Error) {
        ev |= EPOLLERR;
    }
    if(events & EventType::EV_READ) {
        ev |= EPOLLIN;
    }
    if(events & EventType::EV_WRITE) {
        ev |= EPOLLOUT;
    }

    d.ptr = ed;

    std::cout << "in evetns " << (int)ed->events_ << " - " << (int)events << " =" << std::endl;
    std::cout << "in & " << ed << std::endl;

    ep.AddSocket(socket, ev, d);

    ++this->fd_count;

    this->list.push_front(ed);
    return this->list.begin();
}

void WEpoll::ModifySocket(fd_list_item item) {
    uint32_t ev     = 0;
    uint8_t  events = (*item)->events_;

    if(events & EventType::EV_Error) {
        ev |= EPOLLERR;
    }
    if(events & EventType::EV_READ) {
        ev |= EPOLLIN;
    }
    if(events & EventType::EV_WRITE) {
        ev |= EPOLLOUT;
    }

    ep.ModifySocket((*item)->socket_, ev);
}

void WEpoll::DelSocket(WEpoll::fd_list_item item) {
    --this->fd_count;
    this->ep.RemoveSocket((*item)->socket_);
    this->list.erase(item);
    delete *item;
}

void WEpoll::EventLoop() {
    epoll_event *events;

    while(this->active_) {
        int events_size = fd_count;
        std::cout << "array len " << events_size << std::endl;
        events = new epoll_event[events_size];

        events_size = ep.GetEvents(events, events_size, -1);

        if(events_size == -1) {
            std::cout << "error : " << strerror(errno) << std::endl;
            break;
        } else if(events_size == 0) {
            continue;
        }

        for(size_t i = 0; i < events_size; i++) {
            uint32_t         ev   = events[i].events;
            ep_data_t       *data = (ep_data_t *)events[i].data.ptr;
            uint8_t          eev  = data->events_;
            base_socket_type sock = data->socket_;

            std::cout << "out & " << data << std::endl;

            std::cout << ev << " - - " << (int)eev << std::endl;
            std::cout << "socket " << sock << std::endl;
            if(ev & EPOLLIN && eev & EventType::EV_READ) {
                if(this->read_) {
                    this->read_(sock, data->user_data_);
                }
            }
            if(ev & EPOLLOUT && eev & EventType::EV_WRITE) {
                if(this->write_) {
                    this->write_(sock, data->user_data_);
                }
            }
            if(ev & EPOLLERR && eev & EventType::EV_Error) {
                if(this->error_) {
                    this->error_(sock, data->user_data_);
                }
            }
        }

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
