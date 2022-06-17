#include "WNetWork/WSelect.h"

#include <iostream>

namespace wlb::network {

WSelect::WSelect() { this->ClearAllSet(); }
WSelect::~WSelect() { this->ClearAllSet(); }

void WSelect::ClearAllSet() {
    SetClearFd(&read_set_);
    SetClearFd(&write_set_);
}

WSelect::fd_list_item WSelect::NewSocket(base_socket_type socket, uint8_t events, user_data_ptr user_data) {
    if(this->fd_count_ == 1024) {
        return this->list.end();
    }

    auto *sl       = new hdle_data_t;
    sl->socket_    = socket;
    sl->events_    = events;
    sl->user_data_ = user_data;

    this->list.push_front(sl);
    ++this->fd_count_;
    return this->list.begin();
}

void WSelect::ModifySocket(fd_list_item item) { return; }

void WSelect::DelSocket(fd_list_item item) {
    this->rubish_stack_.push(item);
    SetDelFd((*item)->socket_, &read_set_);
    SetDelFd((*item)->socket_, &write_set_);
    --this->fd_count_;
}


void WSelect::Start() {
    this->active_      = true;
    this->work_thread_ = new std::thread(&WSelect::EventLoop, this);
}

void WSelect::Detach() { this->work_thread_->detach(); }

void WSelect::Stop() { this->active_ = false; }

void WSelect::Join() {
    if(this->work_thread_ && this->work_thread_->joinable()) {
        this->work_thread_->join();
    }
}

void WSelect::EventLoop() {
    int res = 0;

    while(this->active_) {
        ClearAllSet();
        SetAllSet();

        res = Select(max_fd_number, &read_set_, &write_set_, nullptr, -1);

        if(res == -1) {
            std::cerr << "error : " << strerror(errno) << std::endl;
            break;
        } else if(res == 0) {
            // time out
            continue;
        }
        std::cout << "select res : " << res << std::endl;

        auto temp_end = this->list.end();
        // 迭代器失效

        for(auto i = this->list.begin(); i != temp_end; ++i) {
            std::cout << "one for start " << this->list.size() << std::endl;
            if(this->read_) {
                std::cout << "has read" << std::endl;
                if((*i)->events_ & KernelEventType::EV_IN && SetCheckFd((*i)->socket_, &read_set_)) {
                    std::cout << "read start" << std::endl;
                    this->read_((*i)->socket_, (*i)->user_data_);
                    std::cout << "read over" << std::endl;
                }
            }
            if(this->write_) {
                std::cout << "has write" << std::endl;
                if((*i)->events_ & KernelEventType::EV_OUT && SetCheckFd((*i)->socket_, &write_set_)) {
                    std::cout << "write start" << std::endl;
                    this->write_((*i)->socket_, (*i)->user_data_);
                    std::cout << "write over" << std::endl;
                }
            }
            std::cout << "one for over " << std::endl;
        }
        std::cout << "one loop over " << std::endl;

        while(!this->rubish_stack_.empty()) {
            this->list.erase(this->rubish_stack_.top());
            this->rubish_stack_.pop();
        }
        std::cout << "rubish over " << this->list.size() << std::endl;
    }
}

void WSelect::SetAllSet() {
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

    ++max_fd_number; // 重要!!!
}

} // namespace wlb::network
