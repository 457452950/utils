#pragma once
#ifndef WUTILS_MESSAGE_QUEUE_MQUEUE_H
#define WUTILS_MESSAGE_QUEUE_MQUEUE_H

#include <queue>
#include <atomic>
#include <list>

namespace wutils {

template <typename T, typename Container = std::queue<T>>
class MQueue {
public:
    MQueue()  = default;
    ~MQueue() = default;

    MQueue(const MQueue &)            = delete;
    MQueue &operator=(const MQueue &) = delete;

    MQueue(const MQueue &&other) noexcept : flag_(std::move(other.flag_)), queue_(std::move(other.queue_)) {}
    MQueue &operator=(MQueue &&) = default;

    void Push(T value) {
        while(flag_.test_and_set()) {
            // pass
        }

        queue_.push(std::forward<T>(value));
        flag_.clear();
    }

    T *Get(T *value) {
        while(flag_.test_and_set()) {
            // pass
        }

        if(queue_.empty()) {
            flag_.clear();
            return nullptr;
        }

        *value = queue_.front();
        queue_.pop();
        flag_.clear();
        return value;
    }

    size_t Size() const { return queue_.size(); }

    bool Empty() const { return queue_.empty(); }

private:
    std::atomic_flag flag_{false};
    Container        queue_;
};

template <typename T>
class MQueue<T, std::list<T>> {
public:
    MQueue()  = default;
    ~MQueue() = default;

    MQueue(const MQueue &)            = delete;
    MQueue &operator=(const MQueue &) = delete;

    MQueue(const MQueue &&other) noexcept : flag_(std::move(other.flag_)), queue_(std::move(other.queue_)) {}
    MQueue &operator=(MQueue &&) = default;

    void Push(T value) {
        while(flag_.test_and_set()) {
            // pass
        }

        queue_.push_back(std::forward<T>(value));
        flag_.clear();
    }

    T *Get(T *value) {
        while(flag_.test_and_set()) {
            // pass
        }

        if(queue_.empty()) {
            flag_.clear();
            return nullptr;
        }

        *value = queue_.front();
        queue_.pop_front();
        flag_.clear();
        return value;
    }

    size_t Size() const { return queue_.size(); }

    bool Empty() const { return queue_.empty(); }

private:
    std::atomic_flag flag_{false};
    std::list<T>     queue_;
};


} // namespace wutils

#endif // WUTILS_MESSAGE_QUEUE_MQUEUE_H
