#pragma once
#ifndef UTILS_WEVENT_H
#define UTILS_WEVENT_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>

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
    virtual ~WEventHandle() {}

    using user_data_type = UserData;
    using user_data_ptr  = user_data_type *;

    class WEventHandler;

    // using option_type      = WEventHandler;
    // using option_list      = std::list<WEventHandler *>;
    // using option_list_item = typename option_list::iterator;

    // call back
    using callback_type = void (*)(base_socket_type sock, user_data_ptr data);

    callback_type read_{nullptr};
    callback_type write_{nullptr};

    // control
    virtual bool AddSocket(WEventHandler *handler)    = 0;
    virtual bool ModifySocket(WEventHandler *handler) = 0;
    virtual void DelSocket(WEventHandler *handler)    = 0;

    // thread control
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


template <typename UserData>
class WEventHandle<UserData>::WEventHandler {
public:
    // TODO: 增加 events_ 修改的监听函数，自动调用ModifySocket
    // TODO: 增加 端信息
    base_socket_type        socket_{-1};         // native socket
    user_data_ptr           user_data_{nullptr}; // user data, void*
    WEventHandle<UserData> *handle_{nullptr};

private:
    uint8_t events_{0}; // HandlerEventType
    bool    enable_{false};

public:
    void Enable();
    void DisEnable();
    bool IsEnable();
    void SetEvents(uint8_t events);
    auto GetEvents();

    static WEventHandler *
    CreateHandler(base_socket_type socket, user_data_ptr user_data, WEventHandle<UserData> *handle);

    ~WEventHandler();
};


template <typename UserData>
inline void WEventHandle<UserData>::WEventHandler::Enable() {
    // events cant be 0
    assert(this->events_);

    if(enable_) {
        return;
    }
    this->enable_ = true;
    this->handle_->AddSocket(this);
}

template <typename UserData>
inline void WEventHandle<UserData>::WEventHandler::DisEnable() {
    if(!enable_) {
        std::cout << "WEventHandle<UserData>::WEventHandler::DisEnable is not enable now " << std::endl;
        return;
    }
    this->handle_->DelSocket(this);
}

template <typename UserData>
inline bool WEventHandle<UserData>::WEventHandler::IsEnable() {
    return this->enable_;
}

template <typename UserData>
inline void WEventHandle<UserData>::WEventHandler::SetEvents(uint8_t events) {
    if(this->events_ != events) {
        this->events_ = events;
        if(this->enable_) {
            this->handle_->ModifySocket(this);
        }
    }
}

template <typename UserData>
inline auto WEventHandle<UserData>::WEventHandler::GetEvents() {
    return this->events_;
}

// clang-format off
template <typename UserData>
typename WEventHandle<UserData>::WEventHandler* 
WEventHandle<UserData>::WEventHandler::CreateHandler(base_socket_type        socket,
                                                     user_data_ptr           user_data,
                                                     WEventHandle<UserData> *handle) {
    auto the = new WEventHandle<UserData>::WEventHandler;

    if(the == nullptr) {
        return nullptr;
    }

    the->socket_    = socket;
    the->user_data_ = user_data;
    the->handle_    = handle;
    return the;
}
// clang-format on

template <typename UserData>
inline WEventHandle<UserData>::WEventHandler::~WEventHandler() {
    ::close(this->socket_);
}

/**
 *
 */

class WBaseChannel;
using event_handle_t  = WEventHandle<WBaseChannel>;
using event_handle_p  = event_handle_t *;
using event_handler_t = event_handle_t::WEventHandler;
using event_handler_p = event_handler_t *;


} // namespace wlb::network


#endif // UTILS_WEVENT_H