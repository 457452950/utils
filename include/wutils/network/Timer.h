#pragma once
#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

#include "IO_Context.h"
#include "base/Timer.h"

namespace wutils::network {

namespace timer {

class Timer : public event::IOInOnly {
public:
    explicit Timer(weak_ptr<event::IO_Context> handle) {
        this->handler_ = make_shared<event::IO_Context::IO_Handle>();

        this->handler_->socket_  = timer_socket_;
        this->handler_->listener = this;
        this->handler_->handle_  = std::move(handle);
        this->handler_->SetEvents(event::EventType::EV_IN);
    }
    ~Timer() override {
        this->Stop();
        this->timer_socket_.Close();
    }

    std::function<void()> OnTime;

    template <typename Rep, typename Period>
    bool Once(const std::chrono::duration<Rep, Period> &rtime) {
        this->timer_socket_.SetOnce(rtime);

        if(!this->handler_->IsEnable()) {
            this->handler_->Enable();
        }

        return true;
    }
    template <typename Rep, typename Period>
    bool Repeat(const std::chrono::duration<Rep, Period> &rtime) {
        this->timer_socket_.SetRepeat(rtime);

        if(!this->handler_->IsEnable()) {
            this->handler_->Enable();
        }

        return true;
    }
    void Stop() {
        if(this->handler_->IsEnable()) {
            std::cout << "Timer::Stop() " << std::endl;
            this->handler_->DisEnable();
        }
    }
    // 定时器是否活跃，即是否已完成定时任务
    bool IsActive() const { return this->handler_->IsEnable(); }

private:
    void EventIn() final {
        this->timer_socket_.Read();

        if(OnTime) {
            OnTime();
        }
    }

private:
    event::IO_Context::IO_Handle_p handler_;
    Socket                         timer_socket_;
};

} // namespace timer

} // namespace wutils::network

#endif // UTIL_TIMER_H
