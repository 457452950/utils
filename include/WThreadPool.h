#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace wlb
{

using user_data_t = union _user_data_t
{
    long long interger;
    void* ptr{nullptr};
};
using task_function_t = std::function<void(user_data_t)>;

struct task
{
    task_function_t _function;
    user_data_t _userData;
    task(const task_function_t& funtion, user_data_t data)
        : _function(funtion), _userData(data) {}
};

using task_list = std::queue<task>;
using thread_list = std::vector<std::thread*>;

class WThreadPool
{
public:
    WThreadPool() {}
    ~WThreadPool() {this->Destroy();}

    WThreadPool(const WThreadPool&) = delete;
    WThreadPool& operator=(const WThreadPool&) = delete;

    bool Start(uint16_t threads_count);
    inline void Stop() {this->_isActive = false;}
    void WaitToStop();
    void Destroy();
    void AddTask(task_function_t function, user_data_t data);

private:
    void ConsumerThread();

private:
    bool _isActive{false};

    uint16_t _threadsCount{1};

    thread_list _threads;
    task_list _tasks;

    std::mutex _mutex;
    std::condition_variable _condition;
};






}






