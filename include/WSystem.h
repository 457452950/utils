#pragma once

#include "WOS.h"

#include "string"

#if OS_IS_WINDOWS

#include <direct.h>     // mkdir
#include <io.h>         // access

#elif OS_IS_LINUX

#include <sys/stat.h>   // mkdir
#include <unistd.h>     // access

#endif // OS_IS_WINDOWS || OS_IS_LINUX




namespace wlb
{

bool IsFileExist(const std::string& path);      // 文件是否存在
bool mkdir(const std::string& path);            // 创建文件


}   // namespace wlb
