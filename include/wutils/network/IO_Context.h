#pragma once
#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <memory>

#include "IO_Event.h"
#include "base/Defined.h"
#include "base/Epoll.h"
#include "base/ISocket.h"
#include "base/Select.h"
#include "wutils/SharedPtr.h"

namespace wutils::network::event {


namespace EventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace EventType

class IO_Context {
public:
    IO_Context()          = default;
    virtual ~IO_Context() = default;

    // noncopyable
    IO_Context(const IO_Context &)            = delete;
    IO_Context &operator=(const IO_Context &) = delete;

    class IO_Handle;
    using IO_Handle_p = shared_ptr<IO_Handle>;
    using Listener    = IOEvent;

    // control
    virtual bool AddSocket(IO_Handle_p handler)    = 0;
    virtual bool ModifySocket(IO_Handle_p handler) = 0;
    virtual void DelSocket(IO_Handle_p handler)    = 0;

    // thread control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};

class IO_Context::IO_Handle : public enable_shared_from_this<IO_Handle> {
public:
    // DONE: 增加 events_ 修改的监听函数，自动调用ModifySocket
    // TODO: 增加 端信息
    ISocket              socket_{INVALID_SOCKET};
    Listener            *listener{nullptr};
    weak_ptr<IO_Context> handle_;


private:
    uint8_t events_{0}; // EventType
    bool    enable_{false};

public:
    void Enable() {
        // events cant be 0
        assert(this->events_);
        assert(!this->handle_.expired());

        if(enable_) {
            return;
        }
        this->enable_ = true;
        this->handle_.lock()->AddSocket(this->shared_from_this());
    }
    void DisEnable() {
        if(this->handle_.expired()) {
            return;
        }

        if(!enable_) {
            return;
        }

        this->handle_.lock()->DelSocket(this->shared_from_this());
        this->enable_ = false;
    }

    bool IsEnable() const { return this->enable_; };

    void SetEvents(uint8_t events) {
        assert(!this->handle_.expired());

        if(this->events_ != events) {
            this->events_ = events;

            if(this->enable_) {
                if(this->events_ != 0)
                    this->handle_.lock()->ModifySocket(this->shared_from_this());
                else {
                    this->enable_ = false;
                    this->handle_.lock()->DelSocket(this->shared_from_this());
                }
            }
        }
    }
    uint8_t GetEvents() const { return this->events_; }

    ~IO_Handle() {
        if(this->IsEnable()) {
            this->DisEnable();
        }
    }
};

////////////////////////// impl //////////////////////

/**
 * EpollContext
 */
class EpollContext final : public IO_Context {
public:
    EpollContext() = default;
    ~EpollContext() override {
        this->Stop();

        ::close(wakeup_fd_);

        ep.Close();
    }

    using IOHandle   = typename IO_Context::IO_Handle;
    using IOHandle_p = shared_ptr<IOHandle>;

    // control
    bool AddSocket(IOHandle_p handler) override;
    bool ModifySocket(IOHandle_p handler) override;
    void DelSocket(IOHandle_p handler) override;

    bool Init() override {
        // DONE: 独立出Init函数
        if(!ep.Init()) {
            // std::cerr << "epoll init failed!" << std::endl;
            return false;
        }

        wakeup_fd_ = eventfd(0, 0);
        if(wakeup_fd_ == -1) {
            return false;
        }

        if(!this->ep.AddSocket(wakeup_fd_, EPOLLIN)) {
            return false;
        }

        return true;
    }

    void Loop() override;
    void Stop() override;
    void Wake();

private:
    static uint32_t ParseToEpollEvent(uint8_t events);
    void            EventLoop();
    void            DelCrash() {
        for(auto &item : ready_to_del_) {
            if(!this->ep.RemoveSocket(item.first->socket_)) {
                return;
            }
            --this->fd_count_;
        }
        this->ready_to_del_.clear();
    }

private:
    epoll::Epoll ep;
    uint32_t     fd_count_{0};
    int          wakeup_fd_{-1}; // event_fd
    bool         active_{false};

    std::map<IOHandle *, IOHandle_p> ready_to_del_;
};
bool EpollContext::AddSocket(IOHandle_p handler) {
    auto it = this->ready_to_del_.find(handler.get());
    if(it != this->ready_to_del_.end()) {
        ep.RemoveSocket(handler->socket_);
        this->ready_to_del_.erase(it);
        --this->fd_count_;
    }

    epoll_data_t ep_data;

    ep_data.ptr = handler.get();
    auto ev     = this->ParseToEpollEvent(handler->GetEvents());

    if(!ep.AddSocket(handler->socket_, ev, ep_data)) {
        return false;
    }

    ++this->fd_count_;

    return true;
}

bool EpollContext::ModifySocket(IOHandle_p handler) {
    //    std::cout << "Mod ISocket " << handler.get() << std::endl;
    epoll_data_t ep_data{.ptr = nullptr};

    ep_data.ptr = handler->listener;
    auto ev     = this->ParseToEpollEvent(handler->GetEvents());

    return ep.ModifySocket(handler->socket_, ev, ep_data);
}

void EpollContext::DelSocket(IOHandle_p handler) {
    //    std::cout << "Del ISocket " << handler.get() << std::endl;
    this->ready_to_del_.insert({handler.get(), handler});
}

void EpollContext::Loop() {
    this->active_ = true;
    this->EventLoop();
}

inline void EpollContext::Stop() {
    this->active_ = false;
    this->Wake();
}

inline void EpollContext::Wake() { eventfd_write(this->wakeup_fd_, 1); }

uint32_t EpollContext::ParseToEpollEvent(uint8_t events) {
    uint32_t ev = 0;
    if(events & EventType::EV_IN) {
        ev |= EPOLLIN;
    }
    if(events & EventType::EV_OUT) {
        ev |= EPOLLOUT;
    }
    return ev;
}

void EpollContext::EventLoop() {

    std::unique_ptr<epoll_event[]> events;

    while(this->active_) {
        int events_size = fd_count_;
        events.reset(new epoll_event[events_size]);

        //        std::cout << "epoll wait !!! " << fd_count_ << std::endl;
        events_size = ep.GetEvents(events.get(), events_size, -1);

        if(!this->active_) {
            std::cout << "close !!!" << std::endl;
            uint64_t cnt;
            eventfd_read(this->wakeup_fd_, &cnt);
            assert(cnt == 1);
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

            uint32_t ev   = events[i].events;
            auto    *data = (IO_Handle *)events[i].data.ptr;

            assert(this->wakeup_fd_ != events[i].data.fd);

            if(this->ready_to_del_.count(data)) {
                break;
            }

            //            std::cout << "data " << data << std::endl;
            assert(data);
            assert(data->socket_ > 0);
            socket_t sock = data->socket_;
            uint8_t  eev  = data->GetEvents();

            // FIXME: io_out 中可能释放 data
            if(ev & EPOLLOUT && eev & EventType::EV_OUT) {
                if(data->listener) {
                    data->listener->EventOut();
                }
            }

            if(this->ready_to_del_.count(data)) {
                break;
            }

            if(ev & EPOLLIN && eev & EventType::EV_IN) {
                if(data->listener) {
                    data->listener->EventIn();
                }
            }
        }

        this->DelCrash();
    }
}

// not thread safe
class SelectContext final : public event::IO_Context {
public:
    SelectContext()           = default;
    ~SelectContext() override = default;

    using IOHandle   = typename IO_Context::IO_Handle;
    using IOHandle_p = shared_ptr<IOHandle>;

    // control
    bool AddSocket(IOHandle_p handler) override;
    bool ModifySocket(IOHandle_p handler) override;
    void DelSocket(IOHandle_p handler) override;
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

    bool HasEventIN(uint8_t events) { return events & event::EventType::EV_IN; }
    bool HasEventOut(uint8_t events) { return events & event::EventType::EV_OUT; }

    void ParseAndSetEvents(socket_t socket, uint8_t events);

private:
    select::fd_set_type read_set_{};
    select::fd_set_type write_set_{};

    std::unordered_set<IOHandle_p> handler_set;
    std::set<IOHandle_p>           rubish_map_;
    uint32_t                       fd_count_{0};
    socket_t                       max_fd_number{0};

    int  wakeup_fd_{-1};
    bool active_{false};
};

bool SelectContext::AddSocket(IOHandle_p handler) {
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


bool SelectContext::ModifySocket(IOHandle_p handler) {
    auto it = handler_set.find(handler);
    if(it != handler_set.end()) { // found
        ParseAndSetEvents(handler->socket_, handler->GetEvents());
        return true;
    }
    return false;
}


void SelectContext::DelSocket(IOHandle_p handler) {
    auto it = this->handler_set.find(handler);
    if(it == this->handler_set.end()) {
        return;
    }

    this->rubish_map_.insert(handler);

    select::SetDelFd(handler->socket_, &read_set_);
    select::SetDelFd(handler->socket_, &write_set_);

    --this->fd_count_;
}


inline bool SelectContext::Init() {
    this->wakeup_fd_ = eventfd(0, 0);
    if(this->wakeup_fd_ == -1) {
        return false;
    }

    ++this->fd_count_;
    return true;
}


void SelectContext::Loop() {
    this->active_ = true;
    this->EventLoop();
}

inline void SelectContext::Wake() { eventfd_write(this->wakeup_fd_, 1); };


void SelectContext::EventLoop() {
    int res = 0;

    while(this->active_) {
        InitAllToSet();

        res = select::SelectWait(max_fd_number, &read_set_, &write_set_, nullptr, -1);

        if(res == -1) {
            std::cerr << "error : " << SystemError::GetSysErrCode() << std::endl;
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
            IOHandle_p i = it.operator*();

            if(this->rubish_map_.count(i)) {
                break;
            }

            if(select::SetCheckFd(i->socket_, &write_set_)) {
                i->listener->EventOut();
            }


            if(this->rubish_map_.count(i)) {
                break;
            }

            if(select::SetCheckFd(i->socket_, &read_set_)) {
                i->listener->EventOut();
            }
        }

        while(!this->rubish_map_.empty()) {
            this->handler_set.erase(this->rubish_map_.begin().operator*());
        }
        this->rubish_map_.clear();
    }
};


void SelectContext::ClearAllSet() {
    select::SetClearFd(&read_set_);
    select::SetClearFd(&write_set_);
};


void SelectContext::InitAllToSet() {
    ClearAllSet();

    max_fd_number = wakeup_fd_;
    for(auto &it : handler_set) {
        if(max_fd_number < it->socket_) {
            max_fd_number = it->socket_;
        }

        std::cout << "set " << it->socket_ << " " << (int)it->GetEvents() << std::endl;
        this->ParseAndSetEvents(it->socket_, it->GetEvents());
        assert(select::SetCheckFd(it->socket_, &read_set_));
    }

    select::SetAddFd(wakeup_fd_, &read_set_);

    // need !!!
    ++max_fd_number;
}


void SelectContext::ParseAndSetEvents(socket_t socket, uint8_t events) {
    if(HasEventIN(events)) {
        select::SetAddFd(socket, &read_set_);
    } else {
        // SetDelFd(socket, &read_set_);
    }
    if(HasEventOut(events)) {
        select::SetAddFd(socket, &write_set_);
    } else {
        // SetDelFd(socket, &write_set_);
    }
}


} // namespace wutils::network::event


#endif // UTILS_EVENT_H