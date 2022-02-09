#include "WOS.h"

#if defined(OS_IS_LINUX)

#include "unistd.h"
#include <sys/timerfd.h>
#include "WNetWork/WTimerHandler.hpp"


namespace wlb
{

// using timerfd = int;
// struct itimerspec
// {
//   struct timespec it_interval;
//   struct timespec it_value;
// };

timerfd CreateNewTimerfd();

enum class SetTimeFlag
{
    REL = 0,    // 相对时间
    ABS = 1,    // 绝对时间
};
bool SetTimerTime(timerfd fd, SetTimeFlag flag, const struct itimerspec* next_time, struct itimerspec* prev_time = nullptr);





////////////////////////////////////////////////////////////////

// class WTimer  使用精度ms
class WTimer
{
public:
    explicit WTimer(WTimerHandler::Listener* listener, WTimerHandler* handler) 
        : _listener(listener), _handler(handler) {};
    WTimer(const WTimer& other) = delete;
    WTimer& operator=(const WTimer& other) = delete;
    virtual ~WTimer() {::close(this->_fd);};

    bool operator==(const timerfd& rhs);
    bool operator!=(const timerfd& rhs);

    bool Start(long time_value, long interval = 0);
protected:
    timerfd _fd{-1};
    bool _active{false};
    WTimerHandler* _handler{nullptr};
    WTimerHandler::Listener* _listener{nullptr};
};

} // namespace wlb

#endif