#pragma once
#ifndef UTILS_WOS_H
#define UTILS_WOS_H

// Windows环境
#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) && not defined(OS_IS_WINDOWS)

#define OS_IS_WINDOWS 1

#elif defined(__linux) || defined(__linux__) && not defined(OS_IS_LINUX)
// Linux环境
#define OS_IS_LINUX 1

#elif defined(__unix__) && not defined(OS_IS_UNIX)

#define OS_IS_UNIX 1

#endif

#endif //UTILS_WOS_H