#pragma once
#ifndef UTIL_SELECTCONTEXT_H
#define UTIL_SELECTCONTEXT_H

#include <tuple>
#include <unordered_set>

#include <cassert>
#include <sys/eventfd.h>

#include "IOContext.h"
#include "wutils/Error.h"
#include "wutils/network/easy/Select.h"

namespace wutils::network::event {


class SelectContext final : public IOContext {
    static inline constexpr eventfd_t WAKE_UP = 1;

    SelectContext() = default;

public:
    ~SelectContext() override {
        delete this->write_set_;
        delete this->read_set_;
    };

    static shared_ptr<SelectContext> Create() { return shared_ptr<SelectContext>(new SelectContext()); }

    // control
    bool AddSocket(IOHandle *handler) override {
        // select fd 上限 1024
        if(this->fd_count_ >= 1024) {
            return false;
        }

        if(this->handler_set_.count(handler)) {
            return true;
        }

        bool ok(true);
        std::tie(std::ignore, ok) = this->handler_set_.insert(handler);
        if(ok) {
            parseAndSetEvents(handler->socket_.Get(), handler->GetEvents());
            ++this->fd_count_;
        }
        return true;
    }
    bool ModifySocket(IOHandle *handler) override {
        auto it = handler_set_.find(handler);
        if(it != handler_set_.end()) { // found
            parseAndSetEvents(handler->socket_.Get(), handler->GetEvents());
            return true;
        }
        return false;
    }
    void DelSocket(IOHandle *handler) override {
        auto it = this->handler_set_.find(handler);
        if(it == this->handler_set_.end()) {
            return;
        }

        this->del_set_.insert(handler);

        SetDelFd(handler->socket_.Get(), read_set_);
        SetDelFd(handler->socket_.Get(), write_set_);
    }

    // thread control
    bool Init() override {
        this->wakeup_fd_ = eventfd(0, 0);
        if(this->wakeup_fd_ == -1) {
            return false;
        }

        ++this->fd_count_;
        return true;
    }
    void Loop() override {
        this->active_ = true;
        this->eventLoop();
    }
    void Stop() override {
        this->active_ = false;
        this->Wake();
    }
    void Wake() { eventfd_write(this->wakeup_fd_, WAKE_UP); }

private:
    void eventLoop() {
        int res = 0;

        while(this->active_) {
            initAllToSet();

            res = SelectWait(int32_t(max_fd_), read_set_, write_set_, nullptr, -1);

            if(res == -1) {
                std::cerr << "error : " << SystemError::GetSysErrCode() << std::endl;
                break;
            } else if(res == 0) {
                // time out
                continue;
            }

            if(!this->active_) {
                std::cout << "close !!!" << std::endl;
                uint64_t cnt;
                eventfd_read(this->wakeup_fd_, &cnt);
                assert(cnt == 1);
                break;
            }

            auto temp_end = this->handler_set_.end();
            // 迭代器失效

            for(auto it = this->handler_set_.begin(); it != temp_end; ++it) {
                IOHandle *i = it.operator*();

                if(!this->handler_set_.count(i)) {
                    break;
                }

                if(SetCheckFd(i->socket_.Get(), write_set_)) {
                    i->listener_->IOOut();
                }

                if(!this->handler_set_.count(i)) {
                    break;
                }

                if(SetCheckFd(i->socket_.Get(), read_set_)) {
                    i->listener_->IOIn();
                }
            }

            this->real_del();
        }
    }

    void real_del() {
        for(auto it : this->del_set_) {
            this->handler_set_.erase(it);
            --this->fd_count_;
        }
        this->del_set_.clear();
    }

    // 清理所有socket
    void clearAllSet() {
        SetClearFd(read_set_);
        SetClearFd(write_set_);
    }
    void initAllToSet() {
        clearAllSet();

        max_fd_ = wakeup_fd_;
        for(auto &it : handler_set_) {
            if(max_fd_ < it->socket_.Get()) {
                max_fd_ = it->socket_.Get();
            }

            this->parseAndSetEvents(it->socket_.Get(), it->GetEvents());
            assert(SetCheckFd(it->socket_.Get(), read_set_));
        }

        SetAddFd(wakeup_fd_, read_set_);

        // need !!!
        ++max_fd_;
    }

    bool hasEventIn(uint8_t events) const { return events & EventType::EV_IN; }
    bool hasEventOut(uint8_t events) const { return events & EventType::EV_OUT; }

    void parseAndSetEvents(socket_t socket, uint8_t events) {
        if(hasEventIn(events)) {
            SetAddFd(socket, read_set_);
        } else {
            // SetDelFd(socket, &read_set_);
        }
        if(hasEventOut(events)) {
            SetAddFd(socket, write_set_);
        } else {
            // SetDelFd(socket, &write_set_);
        }
    }

private:
    fd_set_p read_set_{new fd_set_t};
    fd_set_p write_set_{new fd_set_t};

    std::unordered_set<IOHandle *> handler_set_;
    std::unordered_set<IOHandle *> del_set_;
    uint32_t                       fd_count_{0};
    socket_t                       max_fd_{INVALID_SOCKET};

    int  wakeup_fd_{INVALID_SOCKET};
    bool active_{false};
};

} // namespace wutils::network::event

#endif // UTIL_SELECTCONTEXT_H
