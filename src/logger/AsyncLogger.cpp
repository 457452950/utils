//
// Created by wutils on 2021/9/17.
//

#include "wutils/logger/AsyncLogger.h"
#include <functional>
#include <iostream>

namespace wutils::Log {

Logger *Logger::instance_ = nullptr;

Logger *Logger::getInstance() { return instance_; }

Logger::Logger() = default;

Logger::~Logger() {
    Logger::Stop();
    Logger::Wait2Exit();

    if(file_stream_.is_open()) {
        file_stream_.flush();
        file_stream_.close();
    }
}

void Logger::Init(uint8_t type, LOG_LEVEL level, const std::string &fileName) {
    instance_   = new Logger();
    auto *this_ = instance_;

    // basic config
    this_->log_type_  = type;
    this_->log_level_ = level;

    if(this_->log_type_ & LOG_TYPE::L_FILE) {
        // file log
        this_->base_file_name_ = fileName;

        this_->initFilePath();
    }

    this_->running_ = true;
    this_->thread_  = new(std::nothrow) std::thread(&Logger::Loop, instance_);
}

void Logger::Init(uint8_t type, LOG_LEVEL level, const std::string &fileName, std::unordered_set<std::string> tags) {
    Logger::Init(type, level, fileName);
    Logger::getInstance()->tags_.swap(tags);
    Logger::getInstance()->use_tags_ = true;
}

void Logger::Stop() {
    auto *this_ = Logger::getInstance();

    this_->mutex_.lock();
    if(instance_->running_) {
        instance_->running_ = false;
    }
    this_->mutex_.unlock();

    this_->con_variable_.notify_all();
}

void Logger::Wait2Exit() {
    if(instance_->thread_ != nullptr && instance_->thread_->joinable()) {
        instance_->thread_->join();
    }
}

LOG_LEVEL Logger::GetLogLevel() const { return this->log_level_; }

bool Logger::UseTag() const { return this->use_tags_; }

bool Logger::ContainTag(const std::string &tag) const {
    if(this->use_tags_)
        return this->tags_.count(tag) == 1;
    return true;
}

void Logger::initFilePath() {
    if(file_stream_.is_open()) {
        file_stream_.flush();
        file_stream_.close();
    }

    char name[256];
    GetLogFileName(base_file_name_, name, 256);

    if(!IsFileExist("log")) {
        wutils::mkdir("log");
    }

    file_stream_.open(name, std::ios::out);
}

void Logger::Loop() {
    while(running_ || !log_string_list_.empty()) {
        std::unique_lock<std::recursive_mutex> ulock(mutex_);

        while(log_string_list_.empty()) {
            con_variable_.wait(ulock);
            //
            if(!running_ && log_string_list_.empty()) {
                return;
            }
        }

        const std::string &str = log_string_list_.front();
        log_string_list_.pop_front();

        if(this->log_type_ & LOG_TYPE::L_FILE) {
            file_stream_ << str;
            file_stream_.flush();
        }
        if(this->log_type_ & LOG_TYPE::L_STDOUT) {
            std::cout << str;
            std::cout.flush();
        }

        if((++check_times_ %= max_check_times) == 0) {
            if(getFileSize() >= max_file_size_) {
                initFilePath();
            }
        }
    }
}

Helper Logger::Write(const char *level, const char *tag, const char *file, int lineNo, const char *_func) {
    char head[256];
    MakeMessageHead(file, tag, lineNo, level, _func, head, 256);

    mutex_.lock();
    string_stream_ << head;
    string_stream_ << "[tid:" << std::this_thread::get_id() << "]:";

    return std::make_shared<LogHelper>(this);
}

void Logger::commit() {
    string_stream_ << "\n";

    log_string_list_.push_back(string_stream_.str());

    string_stream_.str("");
    string_stream_.clear();

    mutex_.unlock();
    con_variable_.notify_all();
}

} // namespace wutils::Log
