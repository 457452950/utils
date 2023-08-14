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


class SelectContext final : public IOContextImpl {
    SelectContext() = default;

public:
    ~SelectContext() override{};

    static shared_ptr<SelectContext> Create() { return shared_ptr<SelectContext>(new SelectContext()); }

    // control
    Error RegisterHandle(shared_ptr<IOHandle> handler) override {
        // select fd 上限 1024
        if((this->handles_.size() + 1) >= FD_SETSIZE) {
            return eNetWorkError::CONTEXT_TOO_MUCH_HANDLE;
        }

        assert(this->handles_.find(handler) == this->handles_.end());

        this->handles_.insert(handler);

        parseAndSetEvents(handler->socket_.Get(), handler);
        addUpdateMaxSocket(handler->socket_.Get());

        return eNetWorkError::OK;
    }
    Error ModifyHandle(shared_ptr<IOHandle> handler) override {
        assert(std::find(this->handles_.begin(), this->handles_.end(), handler) != this->handles_.end());

        parseAndSetEvents(handler->socket_.Get(), handler);

        return eNetWorkError::OK;
    }
    void UnregisterHandle(shared_ptr<IOHandle> handler) override {
        auto it = std::find(this->handles_.begin(), this->handles_.end(), handler);
        if(it == this->handles_.end()) {
            return;
        }

        del_handles_.push_back(handler);

        rmUpdateMaxSocket(handler->socket_.Get());

        SetDelFd(handler->socket_.Get(), &read_set_);
        SetDelFd(handler->socket_.Get(), &write_set_);
    }

    // thread control
    bool Init() override {
        if(!control_fd_) {
            return false;
        }
        SetAddFd(control_fd_.Get(), &read_set_);
        this->max_fd_ = control_fd_.Get();
        return true;
    }
    void        Wake(WakeUpEvent ev) override { this->control_fd_.Write(ev); };
    WakeUpEvent Once(std::chrono::milliseconds time_out) override {
        auto r_set = read_set_;
        auto w_set = write_set_;

        auto res = SelectWait(max_fd_ + 1, &r_set, &w_set, nullptr, time_out.count());

        if(res == -1) {
            std::cerr << "error : " << GetGenericError().message() << std::endl;
            return FALSE;
        } else if(res == 0) {
            // time out
            return TIMEOUT;
        }

        if(SetCheckFd(control_fd_.Get(), &r_set)) {
            auto val = control_fd_.Read();
            switch(static_cast<WakeUpEvent>(val)) {
            case QUIT:
                return QUIT;
            default:
                break;
            }
        }

        this->OnReady();

        auto iterator = this->handles_.begin();
        while(iterator != this->handles_.end()) {

            shared_ptr<IOHandle> i = iterator.operator*();
            ++iterator;

            if(SetCheckFd(i->socket_.Get(), &write_set_) && SetCheckFd(i->socket_.Get(), &w_set)) {
                i->IOOut();
            }

            if(SetCheckFd(i->socket_.Get(), &read_set_) && SetCheckFd(i->socket_.Get(), &r_set)) {
                i->IOIn();
            }
        }

        this->real_del();
        return LOOP;
    }

private:
    void parseAndSetEvents(socket_t socket, shared_ptr<IOHandle> handle) {
        if(handle->EnableIn()) {
            SetAddFd(socket, &read_set_);
        } else {
            SetDelFd(socket, &read_set_);
        }
        if(handle->EnableOut()) {
            SetAddFd(socket, &write_set_);
        } else {
            SetDelFd(socket, &write_set_);
        }
    }
    void rmUpdateMaxSocket(socket_t the_new) {
        if(the_new <= this->max_fd_) {
            return;
        }
        this->max_fd_ = INVALID_SOCKET;

        static auto f = [this](shared_ptr<IOHandle> handle) {
            this->max_fd_ = std::max(this->max_fd_, handle->socket_.Get());
        };
        std::for_each(this->handles_.begin(), this->handles_.end(), f);
    }
    void addUpdateMaxSocket(socket_t the_new) {
        if(the_new > this->max_fd_) {
            this->max_fd_ = the_new;
        }
    }
    void real_del() {
        for(auto it : del_handles_) {
            this->handles_.erase(it);
        }
        this->del_handles_.clear();
    }

private:
    fd_set_t read_set_;
    fd_set_t write_set_;

    std::unordered_set<shared_ptr<IOHandle>> handles_;
    std::list<shared_ptr<IOHandle>>          del_handles_; // 确保指针生命

    socket_t max_fd_{INVALID_SOCKET};

    Socket           control_fd_; // event_fd
    std::atomic_bool active_{false};
};

} // namespace wutils::network::event

#endif // UTIL_SELECTCONTEXT_H
