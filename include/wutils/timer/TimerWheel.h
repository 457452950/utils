#pragma once
#ifndef WUTILS_TIMER_TIMERWHEEL_H
#define WUTILS_TIMER_TIMERWHEEL_H

#include <functional>
#include <chrono>
#include <list>

#include "wutils/base/HeadOnly.h"
#include "wutils/SharedPtr.h"

namespace wutils::timer {

class TimeWheel {
    TimeWheel() = default;

public:
    ~TimeWheel() = default;
    static shared_ptr<TimeWheel> Create() { return shared_ptr<TimeWheel>(new TimeWheel); }

    using Task = std::function<void()>;
    // cant be more than 24 hours
    void AddTask(Task &&task, std::chrono::milliseconds ms) {
        using namespace std::chrono_literals;

        if(ms >= 1h) {
            auto n = std::chrono::duration_cast<std::chrono::hours>(ms).count();
            InsertToSorted(hour_wheel_[n], {ms, std::move(task)});
        } else if(ms >= 1min) {
            auto n = std::chrono::duration_cast<std::chrono::minutes>(ms).count();
            InsertToSorted(min_wheel_[n], {ms, std::move(task)});
        } else {
            auto n = std::chrono::duration_cast<std::chrono::seconds>(ms).count();
            InsertToSorted(sec_wheel_[n], {ms, std::move(task)});
        }
    }

    // return next timeout time.
    std::chrono::milliseconds DoTasks();

    std::chrono::milliseconds GetNetTimerTimeOut() {
        using namespace std::chrono_literals;

        auto res = GetMinTaskTimeout(sec_wheel_);
        if(res.count() != 0) {
            return res;
        }

        res = GetMinTaskTimeout(min_wheel_);
        if(res.count() != 0) {
            return res;
        }

        res = GetMinTaskTimeout(hour_wheel_);
        if(res.count() != 0) {
            return res;
        }

        return -1ms;
    }

private:
    static uint64_t CurrentTime() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    struct Task_t {
        Task_t() = default;
        Task_t(std::chrono::milliseconds tt, Task &&t) : ms(tt), task(t) {}
        Task_t(Task_t &&t) = default;

        std::chrono::milliseconds ms{0};
        Task                      task;
    };

    uint64_t current_time_;
    uint64_t pass_;

    std::vector<std::list<Task_t>> hour_wheel_{24};
    std::vector<std::list<Task_t>> min_wheel_{60};
    std::vector<std::list<Task_t>> sec_wheel_{60};

    void InsertToSorted(std::list<Task_t> &list, Task_t &&t) {
        if(list.empty()) {
            list.push_back(std::move(t));
            return;
        }

        auto it = list.begin();
        for(; it != list.end(); ++it) {
            if(t.ms < it->ms) {
                break;
            }
        }
        list.insert(it, std::move(t));
    }

    std::chrono::milliseconds GetMinTaskTimeout(const std::vector<std::list<Task_t>> &vec) {
        for(auto &it : vec) {
            auto res = GetMinTaskTimeout(it);
            if(res.count() > 0) {
                return res;
            }
        }
    }
    std::chrono::milliseconds GetMinTaskTimeout(const std::list<Task_t> &list) {
        using namespace std::chrono_literals;
        if(list.empty()) {
            return 0ms;
        }
        return list.begin()->ms;
    }
};

} // namespace wutils::timer

#endif // WUTILS_TIMER_TIMERWHEEL_H
