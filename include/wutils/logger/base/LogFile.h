#pragma once
#ifndef UTIL_LOGFILE_H
#define UTIL_LOGFILE_H

#include <filesystem>
#include <fstream>
#include <string>

#include "LogBase.h"
#include "wutils/Format.h"
#include "wutils/SharedPtr.h"

namespace fs = std::filesystem;

namespace wutils::log {

class LoggerFile {
public:
    explicit LoggerFile(const std::string &file_name) {
        file_name_ = file_name;
        InitFilePath();
    }
    ~LoggerFile() {
        if(file_.is_open()) {
            file_.flush();
            file_.close();
        }
    }

    void Commit(const std::string &message) {
        file_ << message;
        file_.flush();
    }

private:
    void InitFilePath() {
        fs::path p(this->file_name_);
        if(p.has_parent_path()) {
            auto parent_path = p.parent_path();
            if(!exists(parent_path)) {
                fs::create_directories(parent_path);
            }
        }

        if(file_.is_open()) {
            file_.flush();
            return;
        }

        file_.open(file_name_, std::ios::out);
    }

private:
    std::ofstream file_;
    std::string   file_name_;
};

class LoggerMulFile {
public:
    explicit LoggerMulFile(const std::string &file_name) {
        basic_file_name_ = file_name;
        InitFilePath();
    }
    ~LoggerMulFile() {
        if(file_.is_open()) {
            file_.flush();
            file_.close();
        }
    }

    void Commit(const std::string &message) {
        file_ << message;
        file_.flush();

        ++check_times_;
        if((check_times_ %= max_check_times) == 0) {
            if(getFileSize() >= max_file_size_) {
                InitFilePath();
            }
        }
    }

private:
    void InitFilePath() {
        fs::path    p(this->basic_file_name_);
        std::string extension;
        fs::path    parent_path;
        if(p.has_extension()) {
            extension = p.extension();
            p.replace_extension("");
        }

        if(p.has_parent_path()) {
            parent_path = p.parent_path();
            if(!exists(parent_path)) {
                fs::create_directories(parent_path);
            }
        }

        char name[256]{0};
        GetLogFileName(p.filename(), name, 256);

        if(!extension.empty()) {
            p = parent_path.append(name + extension);
        } else {
            p = parent_path.append(std::string(name) + ".log");
        }

        if(file_.is_open()) {
            file_.flush();
            file_.close();
        }

        file_.open(p, std::ios::out);
    }

    int64_t getFileSize() { return file_.tellp(); }

private:
    std::ofstream file_;
    std::string   basic_file_name_;

    const int64_t max_file_size_{100 * 1024 * 1024}; // 100MB
    const int     max_check_times{10};
    int           check_times_{0};                   // 一定轮次检查文件大小
};

} // namespace wutils::log

#endif // UTIL_LOGFILE_H
