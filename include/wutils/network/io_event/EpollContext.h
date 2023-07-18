#pragma once
#ifndef UTIL_EVENT_EPOLLCONTEXT_H
#define UTIL_EVENT_EPOLLCONTEXT_H

#include <unordered_set>

#include "IOContext.h"
#include "wutils/network/easy/Epoll.h"

namespace wutils::network::event {


class EpollContext final : public IOContext {
    static inline constexpr eventfd_t WAKE_UP = 1;

public:
    EpollContext() = default;
    ~EpollContext() override {
        this->Stop();

        ::close(wakeup_fd_);

        ep.Close();
        this->ready_to_del_.clear();
    }

    // control
    bool AddSocket(IOHandle *handler) override {
        auto it = this->ready_to_del_.find(handler);
        if(it != this->ready_to_del_.end()) {
            this->ready_to_del_.erase(it);
            return this->ModifySocket(handler);
        }

        epoll_data_t ep_data{};

        ep_data.ptr = handler;
        auto ev     = parseToEpollEvent(handler->GetEvents());

        if(!ep.AddSocket(handler->socket_.Get(), ev, ep_data)) {
            return false;
        }

        ++this->fd_count_;
        std::cout << "add socket " << handler->socket_.Get() << std::endl;

        return true;
    }
    bool ModifySocket(IOHandle *handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler;
        auto ev     = parseToEpollEvent(handler->GetEvents());

        return ep.ModifySocket(handler->socket_.Get(), ev, ep_data);
    }
    void DelSocket(IOHandle *handler) override { this->ready_to_del_.insert({handler, handler}); }

    bool Init() override {
        if(!ep.Init()) {
            return false;
        }

        wakeup_fd_ = eventfd(0, 0);
        if(wakeup_fd_ == INVALID_SOCKET) {
            return false;
        }

        this->fd_count_ = 1;
        return this->ep.AddSocket(wakeup_fd_, EPOLLIN);
    }
    void Loop() override {
        this->active_ = true;
        this->eventLoop();
    }
    void Stop() override {
        this->active_ = false;
        this->Wake();
    }
    void Wake() { eventfd_write(this->wakeup_fd_, WAKE_UP); };

private:
    static uint32_t parseToEpollEvent(uint8_t events) {
        uint32_t ev = 0;
        if(events & EventType::EV_IN) {
            ev |= EPOLLIN;
        }
        if(events & EventType::EV_OUT) {
            ev |= EPOLLOUT;
        }
        return ev;
    }
    void eventLoop() {

        std::unique_ptr<epoll_event[]> events;

        while(this->active_) {
            int events_size = fd_count_;
            events.reset(new epoll_event[events_size]);

            events_size = ep.GetEvents(events.get(), events_size, -1);

            if(!this->active_) {
                std::cout << "close !!!" << std::endl;
                eventfd_t cnt;
                eventfd_read(this->wakeup_fd_, &cnt);
                if(cnt == WAKE_UP) {
                    break;
                } else {
                    abort();
                }
                break;
            }

            if(events_size == -1) {
                break;
            } else if(events_size == 0) {
                // TODO:add timeout event
                continue;
            }

            for(int i = 0; i < events_size; ++i) {

                if(!this->active_) {
                    break;
                }

                uint32_t ev      = events[i].events;
                auto    *pHandle = static_cast<IOHandle *>(events[i].data.ptr);

                // check
                assert(pHandle);
                assert(pHandle->socket_.Get() > 0);

                if(this->ready_to_del_.count(pHandle)) {
                    break;
                }

                // get event
                uint8_t eev = pHandle->GetEvents();

                if(ev & EPOLLOUT && eev & EventType::EV_OUT) {
                    pHandle->listener_->IOOut();
                }

                if(this->ready_to_del_.count(pHandle)) {
                    break;
                }

                if(ev & EPOLLIN && eev & EventType::EV_IN) {
                    pHandle->listener_->IOIn();
                }
            }

            this->realDel();
        }
    }
    void realDel() {
        for(auto &item : ready_to_del_) {
            if(!this->ep.RemoveSocket(item->socket_.Get())) {
                return;
            }
            --this->fd_count_;
        }
        this->ready_to_del_.clear();
    }

private:
    network::Epoll ep;
    uint16_t       fd_count_{0};
    int            wakeup_fd_{INVALID_SOCKET}; // event_fd
    bool           active_{false};

    std::unordered_set<IOHandle *> ready_to_del_;
};


} // namespace wutils::network::event

#endif // UTIL_EVENT_EPOLLCONTEXT_H
