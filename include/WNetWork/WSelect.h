#pragma once

#include <iostream>
#include <list>
#include <stack>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <thread>
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
    WSelect() { this->ClearAllSet(); };
    ~WSelect() { this->ClearAllSet(); };

    // control
    typename WEventHandle<UserData>::fd_list_item
    NewSocket(base_socket_type                        socket,
              uint8_t                                 events,
              typename WEventHandle<UserData>::user_data_ptr user_data = nullptr) override {
        if(this->fd_count_ == 1024) {
            return this->list.end();
        }

        auto *sl       = new typename WEventHandle<UserData>::hdle_data_t;
        sl->socket_    = socket;
        sl->events_    = events;
        sl->user_data_ = user_data;

        this->list.push_front(sl);
        ++this->fd_count_;
        return this->list.begin();
    }
    void ModifySocket(typename WEventHandle<UserData>::fd_list_item item) override {}
    void DelSocket(typename WEventHandle<UserData>::fd_list_item item) override {
        std::cout << "Del socket 1" << std::endl;
        this->rubish_stack_.push(item);
        std::cout << "Del socket 2" << std::endl;
        SetDelFd((*item)->socket_, &read_set_);
        SetDelFd((*item)->socket_, &write_set_);
        --this->fd_count_;
    }

    // thread control
    void Start() override {
        this->active_      = true;
        this->work_thread_ = new std::thread(&WSelect::EventLoop, this);
    };
    void Detach() override { this->work_thread_->detach(); };
    void Stop() override { this->active_ = false; };
    void Join() override {
        if(this->work_thread_ && this->work_thread_->joinable()) {
            this->work_thread_->join();
        }
    };

private:
    void EventLoop() {
        int res = 0;

        while(this->active_) {
            ClearAllSet();
            SetAllSet();

            std::cout << "fd count " << fd_count_ << std::endl;
            res = Select(max_fd_number, &read_set_, &write_set_, nullptr, -1);

            if(res == -1) {
                std::cerr << "error : " << strerror(errno) << std::endl;
                break;
            } else if(res == 0) {
                // time out
                continue;
            }
            // std::cout << "select res : " << res << std::endl;

            auto temp_end = this->list.end();
            // 迭代器失效

            for(auto i = this->list.begin(); i != temp_end; ++i) {
                // std::cout << "one for start " << this->list.size() << std::endl;
                if(this->read_) {
                    // // std::cout << "has read" << std::endl;
                    if((*i)->events_ & KernelEventType::EV_IN && SetCheckFd((*i)->socket_, &read_set_)) {
                        // std::cout << "read start" << std::endl;
                        this->read_((*i)->socket_, (*i)->user_data_);
                        // std::cout << "read over" << std::endl;
                    }
                }
                if(this->write_) {
                    // std::cout << "has write" << std::endl;
                    if((*i)->events_ & KernelEventType::EV_OUT && SetCheckFd((*i)->socket_, &write_set_)) {
                        // std::cout << "write start" << std::endl;
                        this->write_((*i)->socket_, (*i)->user_data_);
                        // std::cout << "write over" << std::endl;
                    }
                }
                // std::cout << "one for over " << std::endl;
            }
            // std::cout << "one loop over " << std::endl;

            while(!this->rubish_stack_.empty()) {
                this->list.erase(this->rubish_stack_.top());
                this->rubish_stack_.pop();
            }
            // std::cout << "rubish over " << this->list.size() << std::endl;
        }
    };

    // 清理所有描述符
    void ClearAllSet() {
        SetClearFd(&read_set_);
        SetClearFd(&write_set_);
    };
    void SetAllSet() {
        max_fd_number = 0;

        for(auto i : this->list) {
            if(max_fd_number < i->socket_) {
                max_fd_number = i->socket_;
            }
            if(i->events_ & KernelEventType::EV_IN) {
                SetAddFd(i->socket_, &read_set_);
            }
            if(i->events_ & KernelEventType::EV_OUT) {
                SetAddFd(i->socket_, &write_set_);
            }
        }

        ++max_fd_number; // 重要!!! 同时设置最大fd
    }

private:
    fd_set_type read_set_;
    fd_set_type write_set_;

    typename WEventHandle<UserData>::fd_list list;
    uint32_t                          fd_count_{0};
    base_socket_type                  max_fd_number{0};
    bool                              active_{false};
    std::thread                      *work_thread_{nullptr};

    std::stack<typename WEventHandle<UserData>::fd_list_item> rubish_stack_; // 防止迭代器失效
};


} // namespace wlb::network