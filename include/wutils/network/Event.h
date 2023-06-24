#pragma once
#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "NetWorkUtils.h"
#include "NetworkDef.h"
#include "Socket.h"
#include "wutils/SharedPtr.h"


namespace wutils::network {

namespace event {

namespace EventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace EventType

template <typename UserData>
class IO_Context {
public:
    IO_Context()          = default;
    virtual ~IO_Context() = default;

    // noncopyable
    IO_Context(const IO_Context &)            = delete;
    IO_Context &operator=(const IO_Context &) = delete;

    using user_data_type = UserData;
    using user_data_ptr  = user_data_type *;

    class IO_Handle;
    using EventHandler_p = shared_ptr<IO_Handle>;

    // call back
    using callback_type = std::function<void(socket_t sock, user_data_ptr data)>;

    callback_type read_;
    callback_type write_;

    // control
    virtual bool AddSocket(EventHandler_p handler)    = 0;
    virtual bool ModifySocket(EventHandler_p handler) = 0;
    virtual void DelSocket(EventHandler_p handler)    = 0;

    // thread control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


template <typename UserData>
class IO_Context<UserData>::IO_Handle : public enable_shared_from_this<IO_Handle> {
public:
    // DONE: 增加 events_ 修改的监听函数，自动调用ModifySocket
    // TODO: 增加 端信息
    socket_t                       socket_{-1};         // native socket
    user_data_ptr                  user_data_{nullptr}; // user data, void*
    weak_ptr<IO_Context<UserData>> handle_;


private:
    uint8_t events_{0}; // EventType
    bool    enable_{false};

public:
    void Enable();
    void DisEnable();
    bool IsEnable();

    void SetEvents(uint8_t events);
    auto GetEvents();

    ~IO_Handle();
};


template <typename UserData>
inline void IO_Context<UserData>::IO_Handle::Enable() {
    // events cant be 0
    assert(this->events_);
    assert(!this->handle_.expired());

    if(enable_) {
        return;
    }
    this->enable_ = true;
    this->handle_.lock()->AddSocket(this->shared_from_this());
}

template <typename UserData>
inline void IO_Context<UserData>::IO_Handle::DisEnable() {
    if(this->handle_.expired()) {
        return;
    }

    if(!enable_) {
        return;
    }

    this->handle_.lock()->DelSocket(this->shared_from_this());
    this->enable_ = false;
}

template <typename UserData>
inline bool IO_Context<UserData>::IO_Handle::IsEnable() {
    return this->enable_;
}

template <typename UserData>
inline void IO_Context<UserData>::IO_Handle::SetEvents(uint8_t events) {
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

template <typename UserData>
inline auto IO_Context<UserData>::IO_Handle::GetEvents() {
    return this->events_;
}

template <typename UserData>
inline IO_Context<UserData>::IO_Handle::~IO_Handle() {
    if(this->IsEnable()) {
        this->DisEnable();
    }
}

} // namespace event

} // namespace wutils::network


#endif // UTILS_EVENT_H