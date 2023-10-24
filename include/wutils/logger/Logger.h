#pragma once
#ifndef UTIL_LOGGER_H
#define UTIL_LOGGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include "wutils/logger/base/LogBase.h"
#include "wutils/logger/base/LogFile.h"


namespace wutils::log {

class Logger final {
public:
    static Logger *GetInstance() {
        if(instance_ == nullptr) {
            instance_ = new Logger();
        }
        return instance_;
    }
    Logger(const Logger &)            = delete;
    Logger(Logger &&)                 = delete;
    Logger &operator=(const Logger &) = delete;

private:
    static Logger *instance_;
    Logger() = default;

public:
    Logger *LogCout() {
        this->cout_ = &std::cout;
        return this;
    }

    Logger *LogFile(const std::string &file_name) {
        this->files_.push_back(new LoggerMulFile(file_name));
        return this;
    }

    Logger *SetLogLevel(LOG_LEVEL level) {
        this->log_level_ = level;
        return this;
    }
    LOG_LEVEL GetLogLevel() { return this->log_level_; }

    Logger *UseTags(std::unordered_set<std::string> tags) {
        this->use_tags_ = true;
        this->tags_     = std::move(tags);
        return this;
    }
    bool ContainTag(const std::string &tag) {
        if(this->use_tags_) {
            return this->tags_.count(tag);
        }
        return true;
    }

    Logger *SetCache(int times) {
        this->times_ = times;
        return this;
    }

    void StopAndWait() {
        this->mutex_.lock();
        if(instance_->running_) {
            instance_->running_ = false;
        }
        this->mutex_.unlock();

        this->cv_.notify_all();

        if(this->thread_ != nullptr && this->thread_->joinable()) {
            this->thread_->join();
            delete thread_;
            thread_ = nullptr;
        }

        for(auto it : this->files_) {
            delete it;
        }
        this->files_.clear();
    }

    void Commit(const std::string &message) {
        mutex_.lock();
        buffer_.append(message).append("\n");
        ++times_cur_;
        mutex_.unlock();

        if(times_cur_.load() > times_) {
            cv_.notify_all();
            times_cur_.store(0);
        }
    }

    void Start() {
        this->running_ = true;
        this->thread_  = new(std::nothrow) std::thread(&Logger::Loop, this);
    }

private:
    void Loop() {
        while(running_ || !buffer_.empty()) {
            std::unique_lock<std::recursive_mutex> ulock(mutex_);

            while(buffer_.empty()) {
                using namespace std::chrono;
                cv_.wait_for(ulock, 2ms, [&]() -> bool {
                    if(!running_) {
                        return true;
                    }
                    if(!buffer_.empty()) {
                        return true;
                    }
                    return false;
                });
                //
                if(!running_ && buffer_.empty()) {
                    return;
                }
            }

            if(this->cout_) {
                (*cout_) << buffer_;
                cout_->flush();
            }
            for(auto it : this->files_) {
                it->Commit(buffer_);
            }

            buffer_.clear();
        }
    }

private:
    std::ostream                *cout_{nullptr};
    std::vector<LoggerMulFile *> files_;

    std::string      buffer_;
    int              times_{0};
    std::atomic<int> times_cur_{0};

    // log config
    LOG_LEVEL log_level_{LERROR};

    bool                            use_tags_{false};
    std::unordered_set<std::string> tags_;

    bool running_{false};

    std::thread                *thread_{nullptr};
    std::recursive_mutex        mutex_;
    std::condition_variable_any cv_;
};

Logger *Logger::instance_ = nullptr;

} // namespace wutils::log

#endif // UTIL_LOGGER_H
