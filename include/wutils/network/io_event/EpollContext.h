#pragma once
#ifndef UTIL_EVENT_EPOLLCONTEXT_H
#define UTIL_EVENT_EPOLLCONTEXT_H

#include <unordered_set>

#include "IOContext.h"
#include "wutils/network/easy/Epoll.h"
#include "wutils/network/easy/Event.h"
#include "wutils/SharedPtr.h"

namespace wutils::network::event {


class EpollContext final : public IOContext {
    enum WakeUpEvent : eventfd_t {
        QUIT = 1,
    };

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
    Error AddSocket(IOHandle *handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler;
        auto ev     = parseToEpollEvent(handler);

        if(!ep.AddSocket(handler->socket_.Get(), ev, ep_data)) {
            return GetGenericError();
        }

        ++this->fd_count_;

        return eNetWorkError::OK;
    }
    Error ModifySocket(IOHandle *handler) override {
        epoll_data_t ep_data{};

        ep_data.ptr = handler;
        auto ev     = parseToEpollEvent(handler);

        bool ok = ep.ModifySocket(handler->socket_.Get(), ev, ep_data);
        if(ok) {
            return eNetWorkError::OK;
        } else {
            return GetGenericError();
        }
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
    static uint32_t parseToEpollEvent(IOHandle *handle) {
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

                if(ev & EPOLLOUT && pHandle->EnableOut()) {
                    pHandle->listener_->IOOut();
                }

                if(this->ready_to_del_.count(pHandle)) {
                    break;
                }

                if(ev & EPOLLIN && pHandle->EnableIn()) {
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
