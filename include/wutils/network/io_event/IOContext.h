#pragma once
#ifndef UTILS_IOCONTEXT_H
#define UTILS_IOCONTEXT_H

#include <cassert>
#include <queue>

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


    // TODO:
    using Task = std::function<void()>;

    virtual void Post(Task &&task) = 0;

    std::function<void(Error)> OnError;

    // life control
    virtual bool Init() = 0;
    virtual void Loop() = 0;
    virtual void Stop() = 0;
};


class IOContextImpl : public IOContext {
public:
    IOContextImpl()           = default;
    ~IOContextImpl() override = default;

    // control
    virtual Error RegisterHandle(IOHandle *handler)   = 0;
    virtual Error ModifyHandle(IOHandle *handler)     = 0;
    virtual void  UnregisterHandle(IOHandle *handler) = 0;


protected:
    enum WakeUpEvent : eventfd_t {
        FALSE   = 0,
        QUIT    = 1,
        TIMEOUT = 2,
        LOOP    = 3,
    };

    void Loop() final {
        using namespace std::chrono_literals;

        active_.store(true);

        thread_id_ = std::this_thread::get_id();

        while(active_) {
            auto event = this->Once(-1ms);

            switch(event) {
            case FALSE: {
                if(OnError) {
                    OnError(GetGenericError());
                }
                return;
            }
            case QUIT: {
                return;
            }
            case LOOP: {
            }
            case TIMEOUT: {
                // check timer
                break;
            }
            }

            this->doTasks();
        }
    }
    void Stop() final {
        this->active_.store(false);
        this->Wake(QUIT);
    }

    virtual WakeUpEvent Once(std::chrono::milliseconds time_out) = 0;
    virtual void        Wake(WakeUpEvent ev)                     = 0;

    void OnReady() { assert(isCurrentThread()); }

    void Post(Task &&task) final {
        if(isCurrentThread()) {
            task();
        } else {
            std::unique_lock<std::mutex> _t(this->mutex_);
            this->tasks_.push(std::move(task));
            this->Wake(LOOP);
        }
    }
    bool isCurrentThread() { return this->thread_id_ == std::this_thread::get_id(); }
    void doTasks() {
        assert(isCurrentThread());
        while(!tasks_.empty()) {
            std::unique_lock<std::mutex> _t(this->mutex_);
            tasks_.front()();
            tasks_.pop();
        }
    }

    std::thread::id  thread_id_;
    std::atomic_bool active_{false};

private:
    std::mutex       mutex_; // 保护 tasks
    std::queue<Task> tasks_;
};

class IOHandle {
public:
    using Listener = IOEvent *;

    ISocket                 socket_{INVALID_SOCKET};
    Listener                listener_{nullptr};
    weak_ptr<IOContextImpl> context_;

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
            err = this->context_.lock()->ModifyHandle(this);
            assert(err != eNetWorkError::CONTEXT_CANT_FOUND_HANDLE);
            this->events_ = 0;
            this->enable_ = false;
        }
        // !enable
        else {
            err = this->context_.lock()->RegisterHandle(this);
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

        this->context_.lock()->UnregisterHandle(this);
        this->events_ = 0;
        this->enable_ = false;
    };
};

} // namespace wutils::network::event


#endif // UTILS_IOCONTEXT_H