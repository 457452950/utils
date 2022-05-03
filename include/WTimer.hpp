#pragma once
#include "WOS.h"

#if defined(OS_IS_LINUX)

#include "unistd.h"
#include <sys/timerfd.h>
#include "WNetWork/WTimerHandler.hpp"

namespace wlb {

// using timerfd = int;
// struct itimerspec
// {
//   struct timespec it_interval;
//   struct timespec it_value;
// };

timerfd CreateNewTimerfd();
// 设置时间差的意义
enum class SetTimeFlag {
    REL = 0,    // 相对时间
    ABS = 1,    // 绝对时间
};
bool SetTimerTime(timerfd fd,
                  SetTimeFlag flag,
                  const struct itimerspec *next_time,
                  struct itimerspec *prev_time = nullptr);





////////////////////////////////////////////////////////////////

// class WTimer  使用精度ms

struct WTimerHandlerData;
class WTimerHandler;

// 监听者模式，listener监听者,handler计时器驱动句柄
class WTimer {
public:
    explicit WTimer(WTimerHandler::Listener *listener, WTimerHandler *handler)
            : timerHandler_(handler), listener_(listener) {};
    virtual ~WTimer();;
    // no copyable
    WTimer(const WTimer &other) = delete;
    WTimer &operator=(const WTimer &other) = delete;

    // time_value 初次启动的计时时长，单位为ms, interval循环定时器定时时长，单次定时器设置为0,默认采用相对时间
    bool Start(long time_value, long interval = 0);
    void Stop();
    // 定时器是否活跃，即是否已完成定时任务
    inline bool IsActive() const { return this->active_; }

protected:
    timerfd                 fd_{-1};
    bool                    active_{false};
    WTimerHandler           *timerHandler_{nullptr};
    WTimerHandler::Listener *listener_{nullptr};    // 唯一监听者
    WTimerHandlerData       *timerHandlerData_{nullptr};       // 向定时器驱动句柄的注册信息
};

} // namespace wlb

#endif