#pragma once
#ifndef UTIL_EVENT_EPOLLCONTEXT_H
#define UTIL_EVENT_EPOLLCONTEXT_H

#include <unordered_set>

#include "IOContext.h"
#include "wutils/network/easy/Epoll.h"
#include "wutils/network/easy/Event.h"

namespace wutils::network::event {


class EpollContext final : public IOContext {
    enum WakeUpEvent : eventfd_t {
        QUIT = 1,
    };

public:
    EpollContext() = default;
    ~EpollContext() override {
        this->Stop();

        this->control_fd_.Close();

        ep.Close();
        this->ready_to_del_.clear();
    }

    // control
    bool AddSocket(IOHandle *handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler;
        auto ev     = parseToEpollEvent(handler->GetEvents());

        if(!ep.AddSocket(handler->socket_.Get(), ev, ep_data)) {
            return false;
        }

        ++this->fd_count_;

        return true;
    }
    bool ModifySocket(IOHandle *handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler;
        auto ev     = parseToEpollEvent(handler->GetEvents());

        return ep.ModifySocket(handler->socket_.Get(), ev, ep_data);
    }
    void DelSocket(IOHandle *handler) override {
        this->ep.RemoveSocket(handler->socket_.Get());
        --this->fd_count_;

        this->ready_to_del_.insert(handler);
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
    void Loop() override {
        this->active_.store(true);
        this->eventLoop();
    }
    void Stop() override {
        this->active_.store(false);
        this->Wake(QUIT);
    }
    void Wake(WakeUpEvent ev) { this->control_fd_.Write(ev); };

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
    void realDel() { this->ready_to_del_.clear(); }

    void eventLoop() {

        std::unique_ptr<epoll_event[]> events;

        while(this->active_) {
            int events_size = fd_count_;
            events.reset(new epoll_event[events_size]);

            events_size = ep.GetEvents(events.get(), events_size, -1);

            if(!this->active_) {
                std::cout << "close !!!" << std::endl;
                auto ev = this->control_fd_.Read();
                if(ev == QUIT) {
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

            for(int i = 0; i < events_size && this->active_; ++i) {

                uint32_t ev = events[i].events;

                if(events[i].data.fd == this->control_fd_.Get()) {
                    auto val = this->control_fd_.Read();
                    if(val == QUIT) {
                        break;
                    }
                    continue;
                }

                auto *pHandle = static_cast<IOHandle *>(events[i].data.ptr);

                // check
                assert(pHandle);

                if(this->ready_to_del_.count(pHandle)) {
                    break;
                }

                assert(pHandle->socket_.Get() > 0);

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

private:
    Epoll            ep;
    uint16_t         fd_count_{0};
    Socket           control_fd_; // event_fd
    std::atomic_bool active_{false};

    std::unordered_set<IOHandle *> ready_to_del_;
};


} // namespace wutils::network::event

#endif // UTIL_EVENT_EPOLLCONTEXT_H
