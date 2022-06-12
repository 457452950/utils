#pragma once

#include "WOS.h"

#include "string"

#if OS_IS_WINDOWS

#include <Windows.h>
#include <direct.h> // mkdir
#include <io.h>     // access

#elif OS_IS_LINUX

#include <sys/stat.h> // mkdir
#include <unistd.h>   // access

#endif // OS_IS_WINDOWS || OS_IS_LINUX

namespace wlb {

bool IsFileExist(const std::string &path);    // 文件是否存在
bool mkdir(const std::string &path);          // 创建文件
void GetCurrentTimeFormat(char *buff, int max_len); // 获取当前格式化时间


} // namespace wlb
