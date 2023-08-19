#pragma once
#ifndef WUTILS_TIMER_EASYCONTEXT_H
#define WUTILS_TIMER_EASYCONTEXT_H

#include <set>

#include "Task.h"
#include "wutils/SharedPtr.h"

namespace wutils::timer {
class EasyContext {
    EasyContext() = default;

    using millisec = std::chrono::milliseconds;

public:
    ~EasyContext() = default;
    static shared_ptr<EasyContext> Create() { return shared_ptr<EasyContext>(new EasyContext); }

    // not thread safe
    // 1ms < time out <= 24h
    void AddTask(Task &&task, millisec time_out, Type type = Sync) {
        using namespace std::chrono_literals;
        tasks_.emplace(CurrentTime() + time_out, std::move(task), type);
    }
    void AddTask(const Task &task, millisec time_out, Type type = Sync) {
        using namespace std::chrono_literals;
        tasks_.emplace(CurrentTime() + time_out, task, type);
    }

    /**
     *
     * @return next timer timeout.
     * @note if task set sync, it will run on the thread of DoTask
     */
    millisec DoTasks() {
        using namespace std::chrono_literals;
        current_time_ = CurrentTime();
        while(!tasks_.empty()) {
            auto it = tasks_.begin();
            if(it->ms <= current_time_) {
                it->Do();
            } else {
                return it->ms - current_time_ - 1ms;
            }
            tasks_.erase(it);
        }
        return -1ms;
    }

    int TaskCounts() const noexcept { return tasks_.size(); }

private:
    millisec current_time_{};

    static millisec CurrentTime() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    }

    static bool greater(const Task_t &a, const Task_t &b) { return a.ms > b.ms; }
    static bool less(const Task_t &a, const Task_t &b) { return a.ms < b.ms; }

    std::multiset<Task_t, decltype(&EasyContext::less)> tasks_{&EasyContext::less};
};
} // namespace wutils::timer

#endif // WUTILS_TIMER_EASYCONTEXT_H
