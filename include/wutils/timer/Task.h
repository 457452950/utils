#pragma once
#ifndef WUTILS_TIMER_TIMERWHEEL_H
#define WUTILS_TIMER_TIMERWHEEL_H

#include <functional>
#include <chrono>
#include <thread>
#include <cassert>

namespace wutils::timer {

using Task = std::function<void()>;
enum Type : uint8_t { Sync = 0, Async = 1 };

struct Task_t {
    using millisec = std::chrono::milliseconds;

    Task_t() = default;
    Task_t(std::chrono::milliseconds tt, Task &&t, Type ty) : ms(tt), task(std::move(t)), type(ty) {}
    Task_t(std::chrono::milliseconds tt, const Task &t, Type ty) : ms(tt), task(t), type(ty) {}
    Task_t(const Task_t &)            = default;
    Task_t(Task_t &&)                 = default;
    Task_t &operator=(const Task_t &) = default;

    void Do() const {
        assert(task);
        if(type == Async) {
            std::thread(task).detach();
        } else {
            task();
        }
    }

    millisec ms{0};
    Task     task;
    Type     type{Async};
};


} // namespace wutils::timer

#endif // WUTILS_TIMER_TIMERWHEEL_H
