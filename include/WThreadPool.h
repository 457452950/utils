#pragma once

#include <queue>
#include <utility>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace wlb {

// 任务回调函数参数类型
using user_data_t = union _user_data_t {
    [[maybe_unused]] long long interger;
    [[maybe_unused]] void      *ptr{nullptr};
};
// 任务回调函数类型
using user_function_t = std::function<void(user_data_t)>;

// 任务
struct task {
    user_function_t _function;
    user_data_t     _userData;
    task(user_function_t user_function, user_data_t user_data)
            : _function(std::move(user_function)), _userData(user_data) {}
};

// 线程池
class WThreadPool {
    using task_list = std::queue<task>;             // 任务队列格式
    using thread_list = std::vector<std::thread *>; // 线程队列
public:
    WThreadPool() = default;
    ~WThreadPool() { this->Destroy(); }
    // no copyable
    WThreadPool(const WThreadPool &) = delete;
    WThreadPool &operator=(const WThreadPool &) = delete;

    // lifetime
    // return true if the pool start success
    bool Start(uint16_t threads_count);
    inline void Stop() { this->is_active_ = false; }
    void WaitToStop();  // Wait the pool return (block)
    void Destroy();

    // methods
    void AddTask(const user_function_t &function, user_data_t data);

private:
    void ConsumerThread();      // 消费者线程

private:
    std::atomic_bool        is_active_{false};// 运行
    uint16_t                threads_count_{1};  // 线程数量
    thread_list             threads_;           // 线程
    task_list               tasks_;             // 任务列表
    std::mutex              mutex_;             // 锁
    std::condition_variable condition_;         // 条件变量
};

}






