#pragma once
#ifndef UTILS_IOCONTEXT_H
#define UTILS_IOCONTEXT_H

#include <cassert>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>

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

    // make sure it not cost too much time. it will affect the operation of the main thread.
    using Task = std::function<void()>;

    enum PostMode { Auto = 0, Await = 1 };
    virtual void Post(Task &&task, PostMode mode = Auto) = 0;

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
    Error Register(shared_ptr<IOHandle> handle) {
        if(isCurrentThread()) {
            return this->RegisterHandle(handle);
        } else {
            this->Post([this, handle]() {
                auto err = this->RegisterHandle(handle);
                if(err) {
                    this->handleError(err);
                }
            });
        }
        return {};
    };
    Error Modify(shared_ptr<IOHandle> handle) {
        if(isCurrentThread()) {
            return this->ModifyHandle(handle);
        } else {
            this->Post([this, handle]() {
                auto err = this->ModifyHandle(handle);
                if(err) {
                    this->handleError(err);
                }
            });
        }
        return {};
    }
    void Unregister(shared_ptr<IOHandle> handle) {
        if(isCurrentThread()) {
            this->UnregisterHandle(handle);
        } else {
            this->Post([this, handle]() { this->UnregisterHandle(handle); });
        }
    }


protected:
    enum WakeUpEvent : eventfd_t {
        FALSE   = 0,
        QUIT    = 1,
        TIMEOUT = 2,
        LOOP    = 3,
    };

    // control
    virtual Error RegisterHandle(shared_ptr<IOHandle> handle)   = 0;
    virtual Error ModifyHandle(shared_ptr<IOHandle> handle)     = 0;
    virtual void  UnregisterHandle(shared_ptr<IOHandle> handle) = 0;

    void Loop() final {
        using namespace std::chrono_literals;

        active_.store(true);

        thread_id_ = std::this_thread::get_id();

        while(active_) {
            auto time_out = -1ms;

            if(!this->tasks_.empty()) {
                time_out = 0ms;
            }

            auto event = this->Once(time_out);

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

    void Post(Task &&task, PostMode mode = Auto) final {
        if(isCurrentThread() && mode == Auto) {
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
            this->mutex_.lock();
            auto f = tasks_.front();
            tasks_.pop();
            this->mutex_.unlock();
            f();
        }
    }

    void handleError(Error error) {
        if(OnError) {
            OnError(error);
        }
    }

    std::thread::id  thread_id_;
    std::atomic_bool active_{false};

private:
    std::mutex       mutex_; // 保护 tasks
    std::queue<Task> tasks_;
};

class IOHandle : public enable_shared_from_this<IOHandle> {
public:
    using Listener = IOEvent *;

    ISocket                 socket_{INVALID_SOCKET};
    Listener                listener_{nullptr};
    weak_ptr<IOContextImpl> context_;

    static shared_ptr<IOHandle> Create() { return shared_ptr<IOHandle>(new IOHandle); }

private:
    IOHandle() = default;
    uint8_t events_{0}; // EventType
    bool    enable_{false};

public:
    // you must disable before delete it
    ~IOHandle() { assert(!this->IsEnable()); }

    void IOIn() {
        if(enable_ && this->listener_) {
            this->listener_->IOIn();
        }
    }
    void IOOut() {
        if(enable_ && this->listener_) {
            this->listener_->IOOut();
        }
    }

    bool IsEnable() const { return this->enable_; }
    bool EnableIn() const { return this->events_ & EventType::EV_IN; }
    bool EnableOut() const { return this->events_ & EventType::EV_OUT; }

    // 6 kind of input: All, None, in_only, out_only, dis_in, dis_out.

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
            err = this->context_.lock()->Modify(shared_from_this());
            assert(err != eNetWorkError::CONTEXT_CANT_FOUND_HANDLE);
            this->events_ = 0;
            this->enable_ = false;
        }
        // !enable
        else {
            err = this->context_.lock()->Register(shared_from_this());
            if(err) {
                this->events_ = 0;
            } else {
                this->enable_ = true;
            }
        }
        return err;
    }

    void DisEnable() {
        if(!enable_) {
            return;
        }

        this->events_ = 0;
        this->enable_ = false;

        if(this->context_.expired()) {
            return;
        }
        this->context_.lock()->Unregister(shared_from_this());
    };
};

} // namespace wutils::network::event


#endif // UTILS_IOCONTEXT_H