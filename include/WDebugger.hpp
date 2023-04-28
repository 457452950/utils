#pragma once
#ifndef UTILS_WDEBUGGER_H
#define UTILS_WDEBUGGER_H

#include <atomic>
#include <iostream>
#include <thread>
#include <unordered_map>
#include "WNetWork/WChannel.h"

namespace wlb::debug {

using namespace network;

using WTimerHandle                  = ev_hdle_t;
inline WTimerHandle *debuggerHandle = new WEpoll<WBaseChannel>;

static auto handle_read_callback = [](socket_t sock, WEpoll<WBaseChannel>::user_data_ptr data) {
    auto *ch = (ReadChannel *)data;
    // std::cout << "get channel call channel in [" << ch << "]" << std::endl;
    ch->ChannelIn();
    // std::cout << "get channel call channel in end" << std::endl;
};

class Debugger;
extern Debugger *debugger;

class Debugger final {
public:
    explicit Debugger() = default;
    ~Debugger() { this->Destroy(); };

    // class lifetime
    // init
    bool Init(long timeout) {
        debuggerHandle->read_ = handle_read_callback;

        // this->_timer         = new WTimer(debuggerHandle);
        this->_timer->OnTime = []() {
            // std::cout << "on time" << std::endl;
            for(auto &item : debugger->_memberMap) {
                // std::cout << item.first << " : " << item.second << std::endl;
            }
        };
        this->_isActive = true;
        this->_timer->Start(timeout, timeout);

        // debuggerHandle->Start();
        // debuggerHandle->Detach();
        return true;
    };
    inline bool IsActive() const { return this->_isActive; }
    void        Destroy() {
            //    debuggerHandle->Join();
               delete this->_timer;
               this->_timer = nullptr;
    };

    void MemberAdd(const std::string &name) { _memberMap[name]++; };
    void MemberRemove(const std::string &name) { _memberMap[name]--; };

    WTimer *_timer{nullptr};
    bool    _isActive{false};
    //
    std::unordered_map<std::string, std::atomic<int64_t>> _memberMap;
};

inline Debugger *debugger = new Debugger;
#ifdef DEBUG

#define DEBUGADD(name)                                                                                                 \
    if(debugger != nullptr && debugger->IsActive())                                                                    \
        debugger->MemberAdd(name);

#define DEBUGRM(name)                                                                                                  \
    if(debugger != nullptr && debugger->IsActive())                                                                    \
        debugger->MemberRemove(name);

#define NEWADD DEBUGADD("new")
#define DELADD                                                                                                         \
    DEBUGADD("delete")                                                                                                 \
    DEBUGRM("new")

#define NEWARRAYADD DEBUGADD("new[]")
#define DELARRAYADD                                                                                                    \
    DEBUGADD("delete[]")                                                                                               \
    DEBUGRM("new[]")

#else
#define DEBUGADD(name) ;
#define DEBUGRM(name) ;

#define NEWADD DEBUGADD("new");
#define DELADD ;

#define NEWARRAYADD DEBUGADD("new[]");
#define DELARRAYADD ;

#endif

class WTimeDebugger {
public:
    WTimeDebugger() { this->_stamp = this->GetTime(); }

    void tick() {
        int64_t now = this->GetTime();
        // std::cout << now - this->_stamp << std::endl;
        this->_stamp = now;
    }

private:
    int64_t GetTime() {
        std::chrono::microseconds ms = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch());
        return ms.count();
    };

private:
    int64_t _stamp;
};

[[maybe_unused]] extern WTimeDebugger time_debug;

} // namespace wlb::debug

#endif //UTILS_WDEBUGGER_H
