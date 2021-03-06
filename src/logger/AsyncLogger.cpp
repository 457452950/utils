//
// Created by wlb on 2021/9/17.
//

#include "../../include/AsyncLogger.h"
#include <functional>
#include <iostream>
#include <sys/types.h>

namespace wlb::Log {

Logger *Logger::instance_ = nullptr;

Logger *Logger::getInstance() { return instance_; }

Logger::Logger() = default;

Logger::~Logger() {
    if(file_stream_.is_open()) {
        file_stream_.close();
    }
}

void Logger::initFilePath() {
    if(file_stream_.is_open()) {
        file_stream_.close();
    }

    char name[256];
    wlb::GetLogFileName(Logger::base_file_name_, name, 256);

    if(!IsFileExist("log")) {
        wlb::mkdir("log");
    }
    file_stream_.open(name, std::ios::out);
}

LogHelper_ptr Logger::Write(const char *level, const char *file, int lineNo, const char *_func) {
    char head[256];
    wlb::MakeMessageHead(file, lineNo, level, _func, head, 256);

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

void Logger::Loop() {
    while(running_ || !log_string_list_.empty()) {
        std::unique_lock<std::mutex> ulock(mutex_);
        while(log_string_list_.empty()) {
            con_variable_.wait(ulock);
            //
            if(!running_ && log_string_list_.empty()) {
                std::cout << "out" << std::endl;
                return;
            }
        }
        std::string str = log_string_list_.front();
        log_string_list_.pop_front();

        if(this->log_type_ & LOG_TYPE::L_FILE) {
            file_stream_ << str;
            file_stream_.flush();
        }
        if(this->log_type_ & LOG_TYPE::L_STDOUT) {
            std::cout << str;
        }

        if((++check_times_ %= max_check_times) == 0) {
            if(getFileSize() >= max_file_size_) {
                initFilePath();
            }
        }
    }
}

void Logger::Wait2Exit() {
    if(instance_->thread_ != nullptr && instance_->thread_->joinable()) {
        instance_->thread_->join();
    }
}
void Logger::Stop() {
    auto *this_ = Logger::getInstance();

    this_->mutex_.lock();

    instance_->running_ = false;
    this_->con_variable_.notify_all();

    this_->mutex_.unlock();
}

void Logger::Init(int8_t type, LOG_LEVEL level, char *fileName) {
    instance_   = new Logger();
    auto *this_ = instance_;

    // basic config
    this_->log_type_  = type;
    this_->log_level_ = level;

    if(this_->log_type_ & LOG_TYPE::L_FILE) {
        // file log
        this_->base_file_name_ = new char[strlen(fileName) + 1];
        memcpy(this_->base_file_name_, fileName, strlen(fileName));
        this_->base_file_name_[strlen(fileName)] = '\0';

        this_->initFilePath();
    }

    this_->running_ = true;
    this_->thread_  = new(std::nothrow) std::thread(&Logger::Loop, instance_);
}

LOG_LEVEL Logger::GetLogLevel() { return Logger::getInstance()->log_level_; }

} // namespace wlb::Log
