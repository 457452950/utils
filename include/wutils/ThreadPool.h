#pragma once
#ifndef UTILS_WTHREAD_POOL_H
#define UTILS_WTHREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

namespace wutils {


// 线程池
class ThreadPool {
    // 任务回调函数类型
    using Task_f      = std::function<void(void)>;
    using task_list   = std::queue<Task_f>;         // 任务队列格式
    using thread_list = std::vector<std::thread *>; // 线程队列
public:
    ThreadPool() = default;
    ~ThreadPool() { this->Destroy(); }
    // no copyable
    ThreadPool(const ThreadPool &)            = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    // lifetime
    // return true if the pool start success
    void Start(uint16_t threads_count);
    void Stop() {
        this->is_active_.store(false);
        this->cv_.notify_all();
    }
    // Wait the pool return (block)
    void WaitToStop();
    void Destroy();

    // methods
    void AddTask(Task_f function);

private:
    void ConsumerThread(); // 消费者线程

private:
    std::atomic_bool is_active_{false}; // 运行
    uint16_t         threads_count_{1}; // 线程数量

    thread_list threads_; // 线程
    task_list   tasks_;   // 任务列表

    std::mutex              mutex_; // 锁
    std::condition_variable cv_;    // 条件变量
};

} // namespace wutils

#endif // UTILS_WTHREAD_POOL_H
