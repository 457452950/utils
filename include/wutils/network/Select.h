#pragma once
#ifndef UTILS_SELECT_H
#define UTILS_SELECT_H

#include <sys/select.h>

namespace wutils::network {

// fd_set
using fd_set_t = fd_set;
using fd_set_p = fd_set_t *;

HEAD_ONLY void SetClearFd(fd_set_p set) { FD_ZERO(set); };
HEAD_ONLY void SetAddFd(socket_t fd, fd_set_p set) { FD_SET(fd, set); };
HEAD_ONLY void SetDelFd(socket_t fd, fd_set_p set) { FD_CLR(fd, set); };
HEAD_ONLY bool SetCheckFd(socket_t fd, fd_set_p set) { return FD_ISSET(fd, set); };

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
HEAD_ONLY int32_t
SelectWait(int max_sock, fd_set_p read_set, fd_set_p wirte_set, fd_set_p err_set, int32_t timeout_ms = 0) {

    if(timeout_ms == -1) {
        return ::select(max_sock, read_set, wirte_set, err_set, nullptr);
    }

    timeval time_out{};
    time_out.tv_sec  = timeout_ms / 1000;
    time_out.tv_usec = timeout_ms % 1000;

    return ::select(max_sock, read_set, wirte_set, err_set, &time_out);
}

} // namespace wutils::network


#endif // UTILS_SELECT_H