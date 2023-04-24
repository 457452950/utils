#pragma once
#ifndef UTILS_WSELECT_H
#define UTILS_WSELECT_H


#include <iostream>
#include <list>
#include <stack>
#include <thread>
#include <unordered_set>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/timerfd.h>

#include "WEvent.h"
#include "WNetWorkUtils.h"

namespace wlb::network {

// fd_set
using fd_set_type = fd_set;
using fd_set_ptr  = fd_set_type *;

inline void SetClearFd(fd_set_ptr set) { FD_ZERO(set); };
inline void SetAddFd(base_socket_type fd, fd_set_ptr set) { FD_SET(fd, set); };
inline void SetDelFd(base_socket_type fd, fd_set_ptr set) { FD_CLR(fd, set); };
inline bool SetCheckFd(base_socket_type fd, fd_set_ptr set) { return FD_ISSET(fd, set); };
inline void CopySet(fd_set_ptr set_dist, fd_set_ptr set_src) { memcpy(set_dist, set_src, sizeof(fd_set_type)); }

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
Select(int max_sock, fd_set_ptr read_set, fd_set_ptr wirte_set, fd_set_ptr err_set, int32_t timeout_ms = 0) {

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
class WSelect final : public WEventHandle<UserData> {
public:
    WSelect(){};
    ~WSelect() {
        // this->handler_set.clear();
        // this->rubish_stack_.clear();
    };

    using WEventHandler = typename WEventHandle<UserData>::WEventHandler;

    // control
    bool AddSocket(WEventHandler *handler) override {
        // select fd 上限 1024
        if(this->fd_count_ >= 1024) {
            return false;
        }

        auto res = this->handler_set.insert(handler);
        if(std::get<1>(res) == true) {  // insert succ
            ParseAndSetEvents(handler->socket_, handler->GetEvents());
            ++this->fd_count_;
            return true;
        }
        return false;
    }
    bool ModifySocket(WEventHandler *handler) override {
        auto it = handler_set.find(handler);
        if(it != handler_set.end()) { // found
            ParseAndSetEvents(handler->socket_, handler->GetEvents());
            return true;
        }
        return false;
    }
    void DelSocket(WEventHandler *handler) override {
        auto it = this->handler_set.find(handler);
        if(it == this->handler_set.end()) {
            return;
        }

        this->rubish_stack_.push(handler);

        SetDelFd(handler->socket_, &read_set_);
        SetDelFd(handler->socket_, &write_set_);

        --this->fd_count_;
    }

    // thread control
    void Loop() override {
        this->active_ = true;
        this->EventLoop();
    };
    void Stop() override {
        this->active_ = false;
    }

private:
    void EventLoop() {
        int res = 0;

        while(this->active_) {
            InitAllToSet();

            std::cout << "fd count " << fd_count_ << std::endl;
            res = Select(max_fd_number, &read_set_temp_, &write_set_temp_, nullptr, -1);

            if(res == -1) {
                std::cerr << "error : " << strerror(errno) << std::endl;
                break;
            } else if(res == 0) {
                // time out
                continue;
            }
            // std::cout << "select res : " << res << std::endl;

            auto temp_end = this->handler_set.end();
            // 迭代器失效

            for(auto it = this->handler_set.begin(); it != temp_end; ++it) {
                auto i = *it;

                if(this->write_) {

                    if(SetCheckFd(i->socket_, &write_set_temp_) && SetCheckFd(i->socket_, &write_set_)) {

                        this->write_(i->socket_, i->user_data_);
                    }
                }
                if(this->read_) {

                    if(SetCheckFd(i->socket_, &read_set_temp_) && SetCheckFd(i->socket_, &read_set_)) {

                        this->read_(i->socket_, i->user_data_);
                    }
                }
                // std::cout << "one for over " << std::endl;
            }
            // std::cout << "one loop over " << std::endl;

            while(!this->rubish_stack_.empty()) {
                this->handler_set.erase(this->rubish_stack_.top());
                this->rubish_stack_.pop();
            }
            // std::cout << "rubish over " << this->option_list_.size() << std::endl;
        }
    };

    // 清理所有描述符
    void ClearAllSet() {
        SetClearFd(&read_set_temp_);
        SetClearFd(&write_set_temp_);
    };

    void InitAllToSet() {
        CopySet(&read_set_temp_, &read_set_);
        CopySet(&write_set_temp_, &write_set_);

        max_fd_number = 1;
        for(auto &&it : handler_set) {
            if(max_fd_number < it->socket_) {
                max_fd_number = it->socket_;
            }
        }
    }

    bool HasEventIN(uint8_t events) { return events & HandlerEventType::EV_IN; }
    bool HasEventOut(uint8_t events) { return events & HandlerEventType::EV_OUT; }

    void ParseAndSetEvents(base_socket_type socket, uint8_t events) {
        if(HasEventIN(events)) {
            SetAddFd(socket, &read_set_);
        } else {
            SetDelFd(socket, &read_set_);
        }
        if(HasEventIN(events)) {
            SetAddFd(socket, &write_set_);
        } else {
            SetDelFd(socket, &write_set_);
        }
    }

private:
    fd_set_type read_set_;
    fd_set_type write_set_;
    fd_set_type read_set_temp_;
    fd_set_type write_set_temp_;

    std::unordered_set<WEventHandler *> handler_set;
    std::stack<WEventHandler *>         rubish_stack_;
    uint32_t                            fd_count_{0};
    base_socket_type                    max_fd_number{0};
    bool                                active_{false};
};


} // namespace wlb::network


#endif // UTILS_WSELECT_H