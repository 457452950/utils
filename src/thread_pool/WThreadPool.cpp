#include "../../include/WThreadPool.h"

namespace wlb
{

bool WThreadPool::Start(uint16_t threads_count)
{
    if (this->is_active_)
        return false;
    
    this->threads_count_ = threads_count;

    this->is_active_ = true;
    for (size_t index = 0; index < threads_count; ++index)
    {
        this->threads_.push_back(new std::thread(&WThreadPool::ConsumerThread, this));
    }
    
    return true;
}

void WThreadPool::WaitToStop()
{
    for (auto & thread : this->threads_)
    {
        if (thread->joinable())
        {
            thread->join();
        }
    }
}

void WThreadPool::Destroy()
{
    for (auto & thread : this->threads_)
    {
        delete thread;
    }
    this->threads_.clear();
    
    while (!this->tasks_.empty())
    {
        this->tasks_.pop();
    }
}

void WThreadPool::AddTask(const user_function_t& function, user_data_t user_data)
{
    std::unique_lock<std::mutex> _unique_lock(this->mutex_);
    this->tasks_.emplace(function, user_data);
    this->condition_.notify_all();
}

void WThreadPool::ConsumerThread()
{
    while (this->is_active_)
    {
        std::unique_lock<std::mutex> _unique_lock(this->mutex_);

        if (!this->is_active_)
        {
            break;
        }

        while (this->tasks_.empty())
        {
            if (!this->is_active_)
            {
                return;
            }
            this->condition_.wait(_unique_lock);
        }

        if (!this->is_active_)
        {
            break;
        }

        task _task = std::move(this->tasks_.front());
        this->tasks_.pop();

        _unique_lock.unlock();

        if (_task._function)
        {
            _task._function(_task._userData);
        }
    }
}




}

