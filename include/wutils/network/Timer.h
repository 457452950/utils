#pragma once
#ifndef UTIL_NETWORK_TIMER_H
#define UTIL_NETWORK_TIMER_H

#include <chrono>
#include <functional>

#include "io_event/IOEvent.h"
#include "io_event/IOContext.h"
#include "easy/Timer.h"

namespace wutils::network {

class Timer : public event::IOReadEvent {
private:
    explicit Timer(shared_ptr<event::IOContext> context) : handle_(make_unique<event::IOHandle>()) {
        handle_->listener_ = this;
        handle_->context_  = static_pointer_cast<event::IOContextImpl>(context);
        handle_->socket_   = this->socket_;

        std::cout << "timer socket " << this->socket_.Get() << std::endl;
    }

public:
    static shared_ptr<Timer> Create(shared_ptr<event::IOContext> context) {
        return shared_ptr<Timer>(new Timer(context));
    }
    ~Timer() override {
        this->Stop();

        // release
        handle_.reset();
        this->socket_.Close();
    }

public:
    using OnTime_cb = std::function<void()>;
    OnTime_cb OnTime;

    /**
     *
     * @param first
     * @param loop
     * @param loop_times -1 for no limits, 0 for no loop, number for loop times
     * @return
     */
    template <class Rep1, class Period1, class Rep2 = Rep1, class Period2 = Period1>
    Error Start(const std::chrono::duration<Rep1, Period1> &first,
                const std::chrono::duration<Rep2, Period2> &loop       = std::chrono::duration<Rep1, Period1>(0),
                int32_t                                     loop_times = -1) {
        using namespace std::chrono;

        this->times_ = loop_times;

        ::itimerspec it{};
        auto         sec = duration_cast<seconds>(first);

        it.it_value.tv_sec  = sec.count();
        it.it_value.tv_nsec = duration_cast<nanoseconds>(first - sec).count();

        if(loop_times != 0) {
            sec = duration_cast<seconds>(loop);

            it.it_interval.tv_sec  = sec.count();
            it.it_interval.tv_nsec = duration_cast<nanoseconds>(loop - sec).count();
        }

        auto ok = this->socket_.SetTimeOut(&it);
        if(!ok) {
            return GetGenericError();
        }

        if(!this->handle_->IsEnable()) {
            return handle_->EnableIn(true);
        }
        return eNetWorkError::OK;
    }

    template <class Rep1, class Period1>
    Error Once(const std::chrono::duration<Rep1, Period1> &first) {
        return Start(first);
    }

    template <class Rep1, class Period1>
    Error Loop(const std::chrono::duration<Rep1, Period1> &loop, int32_t times = -1) {
        return Start(loop, loop, times);
    }

    bool IsActive() const { return handle_->IsEnable(); }

    void Stop() {
        if(handle_) {
            if(handle_->IsEnable()) {
                handle_->DisEnable();
            }
        }
    }

private:
    void IOIn() final {
        auto count = this->socket_.Read();

        if(!OnTime) {
            this->Stop();
            return;
        }

        for(uint64_t i = 0; i < count; ++i) {
            if(IsActive()) {
                OnTime();
            }

            if(times_ > 0) {
                --times_;
            } // else if (times == -1) { // no thing to do; }
            if(times_ == 0) {
                this->Stop();
                return;
            }
        }
    }

private:
    unique_ptr<event::IOHandle> handle_;
    timer::Socket               socket_;
    int32_t                     times_{};
};

} // namespace wutils::network

#endif // UTIL_NETWORK_TIMER_H
