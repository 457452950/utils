#pragma once
#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>

#include "Tools.h"
#include "wutils/SharedPtr.h"
#include "wutils/network/base/Native.h"


namespace wutils::network {

namespace HandlerEventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
}; // namespace HandlerEventType

template <typename UserData>
class IOContext {
public:
    IOContext()          = default;
    virtual ~IOContext() = default;

    // noncopyable
    IOContext(const IOContext &)            = delete;
    IOContext &operator=(const IOContext &) = delete;

    using user_data_type = UserData;
    using user_data_ptr  = user_data_type *;

    class IOHandle;
    using IOHandle_p = shared_ptr<IOHandle>;

    // call back
    using callback_type = std::function<void(socket_t sock, user_data_ptr data)>;

    callback_type read_;
    callback_type write_;

    // control
    virtual bool AddSocket(IOHandle_p handler)    = 0;
    virtual bool ModifySocket(IOHandle_p handler) = 0;
    virtual void DelSocket(IOHandle_p handler)    = 0;

    // thread control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


template <typename UserData>
class IOContext<UserData>::IOHandle : public enable_shared_from_this<IOHandle> {
public:
    // DONE: 增加 events_ 修改的监听函数，自动调用ModifySocket
    // TODO: 增加 端信息
    socket_t                           socket_{-1};         // native socket
    user_data_ptr                      user_data_{nullptr}; // user data, void*
    std::weak_ptr<IOContext<UserData>> handle_;


private:
    uint8_t events_{0}; // HandlerEventType
    bool    enable_{false};

public:
    void Enable();
    void DisEnable();
    bool IsEnable();

    void SetEvents(uint8_t events);
    auto GetEvents();

    ~IOHandle();
};


template <typename UserData>
inline void IOContext<UserData>::IOHandle::Enable() {
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
inline void IOContext<UserData>::IOHandle::DisEnable() {
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
inline bool IOContext<UserData>::IOHandle::IsEnable() {
    return this->enable_;
}

template <typename UserData>
inline void IOContext<UserData>::IOHandle::SetEvents(uint8_t events) {
    assert(!this->handle_.expired());

    //    std::cout << "from " << (int)this->events_ << " to " << (int)events << std::endl;
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
inline auto IOContext<UserData>::IOHandle::GetEvents() {
    return this->events_;
}

template <typename UserData>
inline IOContext<UserData>::IOHandle::~IOHandle() {
    if(this->IsEnable()) {
        this->DisEnable();
    }
}


} // namespace wutils::network


#endif // UTILS_EVENT_H