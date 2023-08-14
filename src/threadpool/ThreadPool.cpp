#include "wutils/ThreadPool.h"

#include <cassert>

namespace wutils {

void ThreadPool::Start(uint16_t threads_count) {
    // 检查,不可重复启动
    assert(this->is_active_);

    this->threads_count_ = threads_count;

    this->is_active_ = true;
    for(size_t index = 0; index < threads_count; ++index) {
        this->threads_.push_back(new std::thread(&ThreadPool::ConsumerThread, this));
    }
}

void ThreadPool::WaitToStop() {
    for(auto &thread : this->threads_) {
        if(thread->joinable()) {
            thread->join();
        }
    }
}

void ThreadPool::Destroy() {
    for(auto &thread : this->threads_) {
        delete thread;
    }
    this->threads_.clear();

    while(!this->tasks_.empty()) {
        this->tasks_.pop();
    }
}

void ThreadPool::AddTask(Task_f function) {
    {
        std::unique_lock<std::mutex> _unique_lock(this->mutex_);
        this->tasks_.push(std::move(function));
    }
    this->cv_.notify_all();
}

void ThreadPool::ConsumerThread() {
    while(this->is_active_) {
        // 持锁
        std::unique_lock<std::mutex> _unique_lock(this->mutex_);

        if(!this->is_active_) {
            break;
        }

        while(this->tasks_.empty()) {
            if(!this->is_active_) {
                return;
            }
            this->cv_.wait(_unique_lock);
        }

        if(!this->is_active_) {
            break;
        }

        auto _task = this->tasks_.front();
        this->tasks_.pop();

        _unique_lock.unlock();

        if(_task) {
            _task();
        }
    }
}


} // namespace wutils
