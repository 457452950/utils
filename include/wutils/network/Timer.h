#pragma once
#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include <chrono>
#include <functional>

#include "io_event/IOEvent.h"
#include "io_event/IOContext.h"
#include "easy/Timer.h"

namespace wutils::network {

class Timer : public event::IOReadEvent {
public:
    Timer(weak_ptr<event::IOContext> context) : handle_(make_unique<event::IOHandle>()) {
        handle_->listener_ = this;
        handle_->context_  = std::move(context);
        handle_->socket_   = this->socket_;

        handle_->SetEvents(event::EventType::EV_IN);

        this->socket_.SetNonBlock(true);
    }
    ~Timer() override {
        this->Stop();

        // release
        handle_.reset();
        this->socket_.Close();
    }

public:
    std::function<void()> OnTime;

    /**
     *
     * @param first
     * @param loop
     * @param loop_times -1 for no limits, 0 for no loop, number for loop times
     * @return
     */
    template <class Rep1, class Period1, class Rep2 = Rep1, class Period2 = Period1>
    bool Start(const std::chrono::duration<Rep1, Period1> &first,
               const std::chrono::duration<Rep2, Period2> &loop       = std::chrono::duration<Rep1, Period1>(0),
               int32_t                                     loop_times = -1) {
        using namespace std::chrono;

        this->times_ = loop_times;

        ::itimerspec it{};
        auto         sec    = duration_cast<seconds>(first);

        it.it_value.tv_sec  = sec.count();
        it.it_value.tv_nsec = duration_cast<nanoseconds>(first - sec).count();

        if(loop_times != 0) {
            sec                    = duration_cast<seconds>(loop);

            it.it_interval.tv_sec  = sec.count();
            it.it_interval.tv_nsec = duration_cast<nanoseconds>(loop - sec).count();
        }

        auto ok = this->socket_.SetTimeOut(&it);
        if(!ok) {
            return false;
        }

        if(!this->handle_->IsEnable()) {
            this->handle_->Enable();
        }

        return true;
    }

    template <class Rep1, class Period1>
    bool Once(const std::chrono::duration<Rep1, Period1> &first) {
        return Start(first);
    }

    template <class Rep1, class Period1>
    bool Loop(const std::chrono::duration<Rep1, Period1> &loop, int32_t times = -1) {
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

        if(times_ > 0) {
            --times_;
        } // else if (times == -1) { // no thing to do; }
        if(times_ == 0) {
            this->Stop();
            return;
        }

        if(!OnTime) {
            return;
        }

        for(int i = 0; i < count; ++i) {
            if(IsActive()) {
                OnTime();
            }
        }
    }

private:
    unique_ptr<event::IOHandle> handle_;
    timer::Socket               socket_;
    int32_t                     times_;
};

} // namespace wutils::network

#endif // UTIL_TIMER_H
