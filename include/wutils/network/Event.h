#pragma once
#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "NetWorkDef.h"
#include "NetWorkUtils.h"


namespace wutils::network {

namespace HandlerEventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace HandlerEventType

template <typename UserData>
class EventHandle {
public:
    EventHandle()          = default;
    virtual ~EventHandle() = default;

    // noncopyable
    EventHandle(const EventHandle &)            = delete;
    EventHandle &operator=(const EventHandle &) = delete;

    using user_data_type = UserData;
    using user_data_ptr  = user_data_type *;

    class EventHandler;

    // call back
    using callback_type = void (*)(socket_t sock, user_data_ptr data);

    callback_type read_{nullptr};
    callback_type write_{nullptr};

    // control
    virtual bool AddSocket(EventHandler *handler)    = 0;
    virtual bool ModifySocket(EventHandler *handler) = 0;
    virtual void DelSocket(EventHandler *handler)    = 0;

    // thread control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


template <typename UserData>
class EventHandle<UserData>::EventHandler {
public:
    // DONE: 增加 events_ 修改的监听函数，自动调用ModifySocket
    // TODO: 增加 端信息
    socket_t                             socket_{-1};         // native socket
    user_data_ptr                        user_data_{nullptr}; // user data, void*
    std::weak_ptr<EventHandle<UserData>> handle_;

private:
    uint8_t events_{0}; // HandlerEventType
    bool    enable_{false};

public:
    void Enable();
    void DisEnable();
    bool IsEnable();
    void SetEvents(uint8_t events);
    auto GetEvents();

    ~EventHandler();
};


template <typename UserData>
inline void EventHandle<UserData>::EventHandler::Enable() {
    // events cant be 0
    assert(this->events_);
    assert(!this->handle_.expired());

    if(enable_) {
        return;
    }
    this->enable_ = true;
    this->handle_.lock()->AddSocket(this);
}

template <typename UserData>
inline void EventHandle<UserData>::EventHandler::DisEnable() {
    assert(!this->handle_.expired());
    if(!enable_) {
        std::cout << "EventHandle<UserData>::EventHandler::DisEnable is not enable now " << std::endl;
        return;
    }
    this->handle_.lock()->DelSocket(this);
}

template <typename UserData>
inline bool EventHandle<UserData>::EventHandler::IsEnable() {
    return this->enable_;
}

template <typename UserData>
inline void EventHandle<UserData>::EventHandler::SetEvents(uint8_t events) {
    assert(!this->handle_.expired());

    if(this->events_ != events) {
        this->events_ = events;
        if(this->enable_) {
            this->handle_.lock()->ModifySocket(this);
        }
    }
}

template <typename UserData>
inline auto EventHandle<UserData>::EventHandler::GetEvents() {
    return this->events_;
}

template <typename UserData>
inline EventHandle<UserData>::EventHandler::~EventHandler() {
    ::close(this->socket_);
}


} // namespace wutils::network


#endif // UTILS_EVENT_H