#pragma once

#include <sys/select.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include "WNetWorkUtils.h"

namespace wlb::network {

// fd_set
using fd_set_type = fd_set;
using fd_set_ptr  = fd_set_type *;

inline void SetClearFd(fd_set_ptr set) { FD_ZERO(set); };
inline void SetAddFd(base_socket_type fd, fd_set_ptr set) { FD_SET(fd, set); };
inline void SetDelFd(base_socket_type fd, fd_set_ptr set) { FD_CLR(fd, set); };
inline bool SetCheckFd(base_socket_type fd, fd_set_ptr set) { return FD_ISSET(fd, set); };

// struct timeval
// {
// #ifdef __USE_TIME_BITS64
//   __time64_t tv_sec;		/* Seconds.  */
//   __suseconds64_t tv_usec;	/* Microseconds.  */
// #else
//   __time_t tv_sec;		/* Seconds.  */
//   __suseconds_t tv_usec;	/* Microseconds.  */
// #endif
// };
inline int32_t
Select(int max_sock, fd_set_ptr read_set, fd_set_ptr wirte_set, fd_set_ptr err_set, int32_t timeout_ms = 0) {
    timeval time_out{};
    time_out.tv_sec  = timeout_ms / 1000;
    time_out.tv_usec = timeout_ms % 1000;

    return ::select(max_sock, read_set, wirte_set, err_set, &time_out);
}


} // namespace wlb::network