#pragma once
#ifndef UTIL_EVENT_EPOLLCONTEXT_H
#define UTIL_EVENT_EPOLLCONTEXT_H

#include <unordered_set>

#include "IOContext.h"
#include "wutils/network/easy/Epoll.h"
#include "wutils/network/easy/Event.h"
#include "wutils/SharedPtr.h"

namespace wutils::network::event {


class EpollContext final : public IOContextImpl {
    EpollContext() = default;

public:
    ~EpollContext() override {
        this->Stop();

        this->control_fd_.Close();

        ep.Close();
        this->ready_to_del_.clear();
    }

    static shared_ptr<EpollContext> Create() { return shared_ptr<EpollContext>(new EpollContext()); }

    // control
    Error RegisterHandle(shared_ptr<IOHandle> handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler.get();
        auto ev     = parseToEpollEvent(handler);

        if(!ep.AddSocket(handler->socket_.Get(), ev, ep_data)) {
            return GetGenericError();
        }

        ++this->fd_count_;

        return eNetWorkError::OK;
    }
    Error ModifyHandle(shared_ptr<IOHandle> handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler.get();
        auto ev     = parseToEpollEvent(handler);

        bool ok = ep.ModifySocket(handler->socket_.Get(), ev, ep_data);
        if(ok) {
            return eNetWorkError::OK;
        } else {
            return GetGenericError();
        }
    }
    void UnregisterHandle(shared_ptr<IOHandle> handler) override {
        this->ep.RemoveSocket(handler->socket_.Get());
        --this->fd_count_;

        this->ready_to_del_.push_back(handler);
    }

    bool Init() override {
        if(!ep.Init()) {
            return false;
        }

        if(!control_fd_) {
            return false;
        }

        this->fd_count_ = 1;
        return this->ep.AddSocket(control_fd_.Get(), EPOLLIN);
    }
    void        Wake(WakeUpEvent ev) override { this->control_fd_.Write(ev); };
    WakeUpEvent Once(std::chrono::milliseconds time_out) override {
        resetEpollEventSize();

        auto events_size = ep.GetEvents(epoll_events_.data(), epoll_events_.size(), time_out.count());

        if(!this->active_) {
            std::cout << "close !!!" << std::endl;
            auto ev = this->control_fd_.Read();
            return QUIT;
        }

        if(events_size == -1) {
            return FALSE;
        } else if(events_size == 0) {
            return TIMEOUT;
        }

        this->OnReady();

        for(int i = 0; i < events_size && this->active_; ++i) {

            if(epoll_events_[i].data.fd == this->control_fd_.Get()) {
                auto val = this->control_fd_.Read();
                switch(static_cast<WakeUpEvent>(val)) {
                case QUIT:
                    return QUIT;
                default:
                    break;
                }
                continue;
            }

            uint32_t ev = epoll_events_[i].events;

            auto *pHandle = static_cast<IOHandle *>(epoll_events_[i].data.ptr);

            // check
            assert(pHandle);

            assert(pHandle->socket_.Get() > 0);

            if(ev & EPOLLOUT && pHandle->EnableOut()) {
                pHandle->IOOut();
            }

            if(ev & EPOLLIN && pHandle->EnableIn()) {
                pHandle->IOIn();
            }
        }

        this->realDel();
        return LOOP;
    }

private:
    static uint32_t parseToEpollEvent(shared_ptr<IOHandle> handle) {
        uint32_t ev = 0;
        if(handle->EnableIn()) {
            ev |= EPOLLIN;
        }
        if(handle->EnableOut()) {
            ev |= EPOLLOUT;
        }
        return ev;
    }
    void realDel() { this->ready_to_del_.clear(); }
    void resetEpollEventSize() {
        assert(fd_count_);
        if(std::abs(this->fd_count_ - (uint16_t)this->epoll_events_.size()) > this->fd_count_ / 10) {
            this->epoll_events_.resize(fd_count_);
        }
    }

private:
    Epoll    ep;
    uint16_t fd_count_{0};
    Socket   control_fd_; // event_fd

    std::vector<epoll_event> epoll_events_;

    std::list<shared_ptr<IOHandle>> ready_to_del_;
};


} // namespace wutils::network::event

#endif // UTIL_EVENT_EPOLLCONTEXT_H
