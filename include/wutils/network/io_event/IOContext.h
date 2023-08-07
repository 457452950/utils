#pragma once
#ifndef UTILS_IOCONTEXT_H
#define UTILS_IOCONTEXT_H

#include <cassert>

#include "wutils/SharedPtr.h"
#include "wutils/network/base/ISocket.h"
#include "IOEvent.h"
#include "wutils/network/Error.h"

namespace wutils::network::event {

namespace EventType {
constexpr inline uint8_t EV_IN    = 1 << 0;
constexpr inline uint8_t EV_OUT   = 1 << 1;
constexpr inline uint8_t EV_INOUT = EV_IN | EV_OUT;
}; // namespace EventType

class IOHandle;

class IOContext {
public:
    IOContext()          = default;
    virtual ~IOContext() = default;

    // noncopyable
    IOContext(const IOContext &)            = delete;
    IOContext &operator=(const IOContext &) = delete;

    // control
    virtual Error AddSocket(IOHandle *handler)    = 0;
    virtual Error ModifySocket(IOHandle *handler) = 0;
    virtual void  DelSocket(IOHandle *handler)    = 0;

    // life control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


class IOHandle {
public:
    using Listener = IOEvent *;

    ISocket             socket_{INVALID_SOCKET};
    Listener            listener_{nullptr};
    weak_ptr<IOContext> context_;

private:
    uint8_t events_{0}; // EventType
    bool    enable_{false};

public:
    ~IOHandle() {
        if(this->IsEnable()) {
            this->DisEnable();
        }
    }

    bool IsEnable() const { return this->enable_; }
    bool EnableIn() const { return this->events_ & EventType::EV_IN; }
    bool EnableOut() const { return this->events_ & EventType::EV_OUT; }

    // 6 kind of input, All None in_only out_only dis_in dis_out

    [[nodiscard]] Error EnableAll() {
        // events cant be 0
        assert(this->events_);

        return this->SetEvent(EventType::EV_INOUT);
    }

    [[nodiscard]] Error EnableIn(bool enable) {
        uint8_t event = this->events_;
        if(enable) {
            event |= EventType::EV_IN;
        } else {
            event &= (~EventType::EV_IN);
        }
        return this->SetEvent(event);
    }

    [[nodiscard]] Error EnableOut(bool enable) {
        uint8_t event = this->events_;
        if(enable) {
            event |= EventType::EV_OUT;
        } else {
            event &= (~EventType::EV_OUT);
        }
        return this->SetEvent(event);
    }

    [[nodiscard]] Error SetEvent(uint8_t event) {
        assert(!this->context_.expired());

        if(this->events_ == event) {
            return eNetWorkError::OK;
        }

        if(event == 0) {
            this->DisEnable();
            return eNetWorkError::OK;
        }

        this->events_ = event;

        Error err;
        if(this->enable_) {
            err = this->context_.lock()->ModifySocket(this);
            assert(err != eNetWorkError::CONTEXT_CANT_FOUND_HANDLE);
            this->events_ = 0;
            this->enable_ = false;
        }
        // !enable
        else {
            err = this->context_.lock()->AddSocket(this);
            if(err) {
                this->events_ = 0;
            } else {
                this->enable_ = true;
            }
        }
        return err;
    }

    void DisEnable() {
        if(this->context_.expired()) {
            return;
        }

        if(!enable_) {
            return;
        }

        this->context_.lock()->DelSocket(this);
        this->events_ = 0;
        this->enable_ = false;
    };
};


} // namespace wutils::network::event


#endif // UTILS_IOCONTEXT_H