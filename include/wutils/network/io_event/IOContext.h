#pragma once
#ifndef UTILS_EVENT_H
#define UTILS_EVENT_H

#include "IOEvent.h"
#include "wutils/SharedPtr.h"
#include "wutils/network/base/ISocket.h"
#include <cassert>

namespace wutils::network::event {

namespace EventType {
constexpr inline uint8_t EV_IN  = 1 << 0;
constexpr inline uint8_t EV_OUT = 1 << 1;
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
    virtual bool AddSocket(IOHandle *handler)    = 0;
    virtual bool ModifySocket(IOHandle *handler) = 0;
    virtual void DelSocket(IOHandle *handler)    = 0;

    // life control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


class IOHandle {
public:
    using Observer = IOEvent *;

    ISocket             socket_{INVALID_SOCKET};
    Observer            observer_{nullptr};
    weak_ptr<IOContext> context_;

private:
    uint8_t events_{0}; // EventType
    bool    enable_{false};

public:
    void Enable() {
        // events cant be 0
        assert(this->events_ != 0);
        assert(!this->context_.expired());

        if(enable_) {
            return;
        }
        this->context_.lock()->AddSocket(this);
        this->enable_ = true;
    }
    void DisEnable() {
        if(this->context_.expired()) {
            return;
        }

        if(!enable_) {
            return;
        }

        this->context_.lock()->DelSocket(this);
        this->enable_ = false;
    };
    bool IsEnable() const { return this->enable_; }

    /**
     * set events , will disenable when events is none , will update when events changed
     * @param events the new events
     */
    void SetEvents(uint8_t events) {
        assert(!this->context_.expired());

        if(this->events_ != events) {
            this->events_ = events;

            if(this->enable_) {
                if(this->events_ != 0)
                    this->context_.lock()->ModifySocket(this);
                else {
                    this->enable_ = false;
                    this->context_.lock()->DelSocket(this);
                }
            }
        }
    }
    uint8_t GetEvents() const { return this->events_; }

    ~IOHandle() {
        if(this->IsEnable()) {
            this->DisEnable();
        }
    }
};


} // namespace wutils::network::event


#endif // UTILS_EVENT_H