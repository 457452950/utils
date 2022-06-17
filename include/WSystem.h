#pragma once
#include "string"

#include <sys/stat.h> // mkdir
#include <unistd.h>   // access


namespace wlb {

bool IsFileExist(const std::string &path);          // 文件是否存在
bool mkdir(const std::string &path);                // 创建文件
void GetCurrentTimeFormat(char *buff, int max_len); // 获取当前格式化时间


} // namespace wlb
