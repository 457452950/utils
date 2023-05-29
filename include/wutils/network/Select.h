#pragma once
#ifndef UTILS_SELECT_H
#define UTILS_SELECT_H


#include <iostream>
#include <list>
#include <stack>
#include <thread>
#include <unordered_set>
#include <vector>

#include <sys/eventfd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/timerfd.h>

#include "Event.h"
#include "NetWorkUtils.h"

namespace wutils::network {

// fd_set
using fd_set_type = fd_set;
using fd_set_ptr  = fd_set_type *;

inline void SetClearFd(fd_set_ptr set) { FD_ZERO(set); };
inline void SetAddFd(socket_t fd, fd_set_ptr set) { FD_SET(fd, set); };
inline void SetDelFd(socket_t fd, fd_set_ptr set) { FD_CLR(fd, set); };
inline bool SetCheckFd(socket_t fd, fd_set_ptr set) { return FD_ISSET(fd, set); };

// struct timeval
// {
// #ifdef __USE_TIME_BITS64
//   __time64_t tv_sec;		/* Seconds.  */
//   __suseconds64_t tv_usec;	/* Microseconds.  */
// #else
//   __time_t tv_sec;		/* Seconds.  */
//   __suseconds_t tv_usec;	/* Microseconds.  */
// #endif
// };
inline int32_t
SelectWait(int max_sock, fd_set_ptr read_set, fd_set_ptr wirte_set, fd_set_ptr err_set, int32_t timeout_ms = 0) {

    if(timeout_ms == -1) {
        return ::select(max_sock, read_set, wirte_set, err_set, nullptr);
    }

    timeval time_out{};
    time_out.tv_sec  = timeout_ms / 1000;
    time_out.tv_usec = timeout_ms % 1000;

    return ::select(max_sock, read_set, wirte_set, err_set, &time_out);
}

// not thread safe
template <typename UserData>
class Select final : public EventHandle<UserData> {
public:
    Select()  = default;
    ~Select() = default;

    using EventHandler = typename EventHandle<UserData>::EventHandler;

    // control
    bool AddSocket(EventHandler *handler) override;
    bool ModifySocket(EventHandler *handler) override;
    void DelSocket(EventHandler *handler) override;
    // thread control
    bool Init() override;
    void Loop() override;
    void Stop() override {
        this->active_ = false;
        this->Wake();
    }
    void Wake();

private:
    void EventLoop();

    // 清理所有描述符
    void ClearAllSet();
    void InitAllToSet();

    bool HasEventIN(uint8_t events) { return events & HandlerEventType::EV_IN; }
    bool HasEventOut(uint8_t events) { return events & HandlerEventType::EV_OUT; }

    void ParseAndSetEvents(socket_t socket, uint8_t events);

private:
    fd_set_type read_set_{};
    fd_set_type write_set_{};

    std::unordered_set<EventHandler *> handler_set;
    std::stack<EventHandler *>         rubish_stack_;
    uint32_t                           fd_count_{0};
    socket_t                           max_fd_number{0};

    int  wakeup_fd_{-1};
    bool active_{false};
};


template <typename UserData>
bool Select<UserData>::AddSocket(EventHandler *handler) {
    // select fd 上限 1024
    if(this->fd_count_ >= 1024) {
        return false;
    }

    auto res = this->handler_set.insert(handler);
    if(std::get<1>(res) == true) { // insert succ
        ParseAndSetEvents(handler->socket_, handler->GetEvents());
        ++this->fd_count_;
        return true;
    }
    return false;
}

template <typename UserData>
bool Select<UserData>::ModifySocket(EventHandler *handler) {
    auto it = handler_set.find(handler);
    if(it != handler_set.end()) { // found
        ParseAndSetEvents(handler->socket_, handler->GetEvents());
        return true;
    }
    return false;
}

template <typename UserData>
void Select<UserData>::DelSocket(EventHandler *handler) {
    auto it = this->handler_set.find(handler);
    if(it == this->handler_set.end()) {
        return;
    }

    this->rubish_stack_.push(handler);

    SetDelFd(handler->socket_, &read_set_);
    SetDelFd(handler->socket_, &write_set_);

    --this->fd_count_;
}


template <typename UserData>
inline bool Select<UserData>::Init() {
    this->wakeup_fd_ = eventfd(0, 0);
    if(this->wakeup_fd_ == -1) {
        return false;
    }

    ++this->fd_count_;
    return true;
}

template <typename UserData>
void Select<UserData>::Loop() {
    this->active_ = true;
    this->EventLoop();
}
template <typename UserData>
inline void Select<UserData>::Wake() {
    eventfd_write(this->wakeup_fd_, 1);
};

template <typename UserData>
void Select<UserData>::EventLoop() {
    int res = 0;

    while(this->active_) {
        InitAllToSet();

        res = SelectWait(max_fd_number, &read_set_, &write_set_, nullptr, -1);

        if(res == -1) {
            std::cerr << "error : " << strerror(errno) << std::endl;
            break;
        } else if(res == 0) {
            // time out
            continue;
        }

        if(this->active_ == false) {
            std::cout << "close !!!" << std::endl;
            uint64_t cnt;
            eventfd_read(this->wakeup_fd_, &cnt);
            assert(cnt == 1);
            break;
        }

        auto temp_end = this->handler_set.end();
        // 迭代器失效

        for(auto it = this->handler_set.begin(); it != temp_end; ++it) {
            EventHandler *i = it.operator*();

            if(this->write_) {
                if(SetCheckFd(i->socket_, &write_set_)) {
                    this->write_(i->socket_, i->user_data_);
                }
            }
            if(this->read_) {
                if(SetCheckFd(i->socket_, &read_set_)) {
                    this->read_(i->socket_, i->user_data_);
                }
            }
        }

        while(!this->rubish_stack_.empty()) {
            this->handler_set.erase(this->rubish_stack_.top());
            this->rubish_stack_.pop();
        }
    }
};

template <typename UserData>
void Select<UserData>::ClearAllSet() {
    SetClearFd(&read_set_);
    SetClearFd(&write_set_);
};

template <typename UserData>
void Select<UserData>::InitAllToSet() {
    ClearAllSet();

    max_fd_number = wakeup_fd_;
    for(auto &it : handler_set) {
        if(max_fd_number < it->socket_) {
            max_fd_number = it->socket_;
        }

        std::cout << "set " << it->socket_ << " " << (int)it->GetEvents() << std::endl;
        this->ParseAndSetEvents(it->socket_, it->GetEvents());
        assert(SetCheckFd(it->socket_, &read_set_));
    }

    SetAddFd(wakeup_fd_, &read_set_);

    // need !!!
    ++max_fd_number;
}

template <typename UserData>
void Select<UserData>::ParseAndSetEvents(socket_t socket, uint8_t events) {
    if(HasEventIN(events)) {
        SetAddFd(socket, &read_set_);
    } else {
        // SetDelFd(socket, &read_set_);
    }
    if(HasEventOut(events)) {
        SetAddFd(socket, &write_set_);
    } else {
        // SetDelFd(socket, &write_set_);
    }
}

} // namespace wutils::network


#endif // UTILS_SELECT_H