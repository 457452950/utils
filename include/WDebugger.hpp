#pragma once

#include <atomic>
#include <unordered_map>
#include <thread>
#include "WOS.h"
#include "WTimer.hpp"
#include "WNetWork/WEpoll.hpp"

namespace wlb::debug
{

extern WTimerHandler* debuggerHandler;

class Debugger : public WTimerHandler::Listener
{
public:
    explicit Debugger() {};
    ~Debugger() {this->Destroy();};

    // class lifetime
    // init
    bool Init(long timeout); 
    inline bool IsActive() const {return this->_isActive;}
    void Destroy();

    void MemberAdd(const std::string& name);
    void MemberRemove(const std::string& name);

private:
    // override methods OnTime from WTimerHandler::Listener
    void OnTime(timerfd id) override;
    void Loop();

private:
    WTimer* _timer{nullptr};
    bool _isActive{false};
    std::thread* _thread{nullptr};

    std::unordered_map<std::string, std::atomic<int64_t>> _memberMap;
};

extern Debugger* debugger;

#define DEBUGADD(name) \
    if (debugger != nullptr && debugger->IsActive()) \
        debugger->MemberAdd(name);
        
#define DEBUGRM(name) \
    if (debugger != nullptr && debugger->IsActive()) \
        debugger->MemberRemove(name);

}   // namespace wlb::debug


