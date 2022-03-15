#pragma once

#include <queue>
#include <utility>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace wlb
{

using user_data_t = union _user_data_t
{
    long long interger;
    void* ptr{nullptr};
};
using user_function_t = std::function<void(user_data_t)>;

struct task
{
    user_function_t _function;
    user_data_t _userData;
    task(user_function_t  user_function, user_data_t user_data)
        : _function(std::move(user_function)), _userData(user_data) {}
};

using task_list = std::queue<task>;
using thread_list = std::vector<std::thread*>;

class WThreadPool
{
public:
    WThreadPool() = default;
    ~WThreadPool() {this->Destroy();}

    WThreadPool(const WThreadPool&) = delete;
    WThreadPool& operator=(const WThreadPool&) = delete;

    bool Start(uint16_t threads_count);
    inline void Stop() {this->is_active_ = false;}
    void WaitToStop();
    void Destroy();
    void AddTask(const user_function_t& function, user_data_t data);

private:
    void ConsumerThread();

private:
    std::atomic_bool is_active_{false};

    uint16_t threads_count_{1};

    thread_list threads_;
    task_list tasks_;

    std::mutex mutex_;
    std::condition_variable condition_;
};






}






