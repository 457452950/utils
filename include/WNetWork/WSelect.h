#pragma once

#include <list>
#include <stack>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <thread>
#include "WEvent.h"
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

    if(timeout_ms == -1) {
        return ::select(max_sock, read_set, wirte_set, err_set, nullptr);
    }

    timeval time_out{};
    time_out.tv_sec  = timeout_ms / 1000;
    time_out.tv_usec = timeout_ms % 1000;

    return ::select(max_sock, read_set, wirte_set, err_set, &time_out);
}

// not thread safe
class WSelect final : public WEventHandle {
public:
    WSelect();
    ~WSelect();

    // control
    fd_list_item NewSocket(base_socket_type socket, uint8_t events, user_data_ptr user_data = nullptr) override;
    void         ModifySocket(fd_list_item item) override;
    void         DelSocket(fd_list_item item) override;

    // thread control
    void Start() override;
    void Detach() override;
    void Stop() override;
    void Join() override;

private:
    void EventLoop();

    void ClearAllSet(); // 清理所有描述符
    void SetAllSet();   // 同时设置最大fd

private:
    fd_set_type read_set_;
    fd_set_type write_set_;

    fd_list          list;
    uint32_t         fd_count_{0};
    base_socket_type max_fd_number{0};
    bool             active_{false};
    std::thread     *work_thread_{nullptr};

    std::stack<fd_list_item> rubish_stack_; // 防止迭代器失效
};


} // namespace wlb::network