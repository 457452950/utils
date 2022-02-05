#pragma once

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)

#define OS_IS_WINDOWS 1

#elif defined(__linux) || defined(__linux__)

#define OS_IS_LINUX 1

#endif
