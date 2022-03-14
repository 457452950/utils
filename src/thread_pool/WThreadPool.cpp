#include "../../include/WThreadPool.h"

namespace wlb
{

bool WThreadPool::Start(uint16_t threads_count)
{
    if (this->_isActive)
        return false;
    
    this->_threadsCount = threads_count;

    this->_isActive = true;
    for (size_t index = 0; index < threads_count; ++index)
    {
        this->_threads.push_back(new std::thread(&WThreadPool::ConsumerThread, this));
    }
    
    return true;
}

void WThreadPool::WaitToStop()
{
    for (std::size_t index = 0; index < this->_threads.size(); ++index)
    {
        if (this->_threads[index]->joinable())
        {
            this->_threads[index]->join();
        }
        
    }
}

void WThreadPool::Destroy()
{
    for (size_t index = 0; index < this->_threads.size(); ++index)
    {
        delete this->_threads[index];
    }
    this->_threads.clear();
    
    while (!this->_tasks.empty())
    {
        this->_tasks.pop();
    }
}

void WThreadPool::AddTask(task_function_t function, user_data_t user_data)
{
    std::unique_lock<std::mutex> _unique_lock(this->_mutex);
    this->_tasks.emplace(function, user_data);
    this->_condition.notify_all();
}

void WThreadPool::ConsumerThread()
{
    while (this->_isActive)
    {
        std::unique_lock<std::mutex> _unique_lock(this->_mutex);

        if (!this->_isActive)
        {
            break;
        }

        while (this->_tasks.empty())
        {
            if (!this->_isActive)
            {
                return;
            }
            this->_condition.wait(_unique_lock);
        }

        if (!this->_isActive)
        {
            break;
        }

        task _task = std::move(this->_tasks.front());
        this->_tasks.pop();

        _unique_lock.unlock();

        if (_task._function)
        {
            _task._function(_task._userData);
        }
    }
}




}

