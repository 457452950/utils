#pragma once
#ifndef UTILS_WEVENT_H
#define UTILS_WEVENT_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "WNetWorkDef.h"
#include "WNetWorkUtils.h"


namespace wlb::network {

namespace HandlerEventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace HandlerEventType

template <typename UserData>
class WEventHandle {
public:
    WEventHandle() {}
    virtual ~WEventHandle() {}

    // nocopyable
    WEventHandle(const WEventHandle &)             = delete;
    WEventHandle &operator=(const WEventHandle &) = delete;

    using user_data_type = UserData;
    using user_data_ptr  = user_data_type *;

    class WEventHandler;

    // call back
    using callback_type = void (*)(socket_t sock, user_data_ptr data);

    callback_type read_{nullptr};
    callback_type write_{nullptr};

    // control
    virtual bool AddSocket(WEventHandler *handler)    = 0;
    virtual bool ModifySocket(WEventHandler *handler) = 0;
    virtual void DelSocket(WEventHandler *handler)    = 0;

    // thread control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


template <typename UserData>
class WEventHandle<UserData>::WEventHandler {
public:
    // DONE: 增加 events_ 修改的监听函数，自动调用ModifySocket
    // TODO: 增加 端信息
    socket_t                              socket_{-1};         // native socket
    user_data_ptr                         user_data_{nullptr}; // user data, void*
    std::weak_ptr<WEventHandle<UserData>> handle_;

private:
    uint8_t events_{0}; // HandlerEventType
    bool    enable_{false};

public:
    void Enable();
    void DisEnable();
    bool IsEnable();
    void SetEvents(uint8_t events);
    auto GetEvents();

    ~WEventHandler();
};


template <typename UserData>
inline void WEventHandle<UserData>::WEventHandler::Enable() {
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
inline void WEventHandle<UserData>::WEventHandler::DisEnable() {
    assert(!this->handle_.expired());
    if(!enable_) {
        std::cout << "WEventHandle<UserData>::WEventHandler::DisEnable is not enable now " << std::endl;
        return;
    }
    this->handle_.lock()->DelSocket(this);
}

template <typename UserData>
inline bool WEventHandle<UserData>::WEventHandler::IsEnable() {
    return this->enable_;
}

template <typename UserData>
inline void WEventHandle<UserData>::WEventHandler::SetEvents(uint8_t events) {
    assert(!this->handle_.expired());

    if(this->events_ != events) {
        this->events_ = events;
        if(this->enable_) {
            this->handle_.lock()->ModifySocket(this);
        }
    }
}

template <typename UserData>
inline auto WEventHandle<UserData>::WEventHandler::GetEvents() {
    return this->events_;
}

// // clang-format off
// template <typename UserData>
// std::unique_ptr<typename WEventHandle<UserData>::WEventHandler>
// WEventHandle<UserData>::WEventHandler::CreateHandler(socket_t        socket,
//                                                      user_data_ptr           user_data,
//                                                      std::weak_ptr<WEventHandle<UserData>> handle) {
//     auto the = std::make_unique<WEventHandle<UserData>::WEventHandler>();

//     if(!the) {
//         return nullptr;
//     }

//     the->socket_    = socket;
//     the->user_data_ = user_data;
//     the->handle_    = handle;
//     return the;
// }
// // clang-format on

template <typename UserData>
inline WEventHandle<UserData>::WEventHandler::~WEventHandler() {
    ::close(this->socket_);
}


} // namespace wlb::network


#endif // UTILS_WEVENT_H