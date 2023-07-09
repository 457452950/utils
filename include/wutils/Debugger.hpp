#pragma once
#ifndef UTILS_DEBUGGER_H
#define UTILS_DEBUGGER_H

#include "network/IO_Event.h"
#include <atomic>
#include <iostream>
#include <thread>
#include <unordered_map>

namespace wutils::debug {
//
// using namespace network;
//
// using TimerHandle                  = ev_hdle_t;
// inline TimerHandle *debuggerHandle = new Epoll<IOEvent>;
//
// static auto handle_read_callback = [](native_socket_t sock, Epoll<IOEvent>::user_data_ptr data) {
//    auto *ch = (IOInOnly *)data;
//    ch->EventIn();
//};
//
// class Debugger;
// extern Debugger *debugger;
//
// class Debugger final {
// public:
//    explicit Debugger() = default;
//    ~Debugger() { this->Destroy(); };
//
//    // class lifetime
//    // init
//    bool Init(long timeout) {
//        debuggerHandle->io_in = handle_read_callback;
//
//        // this->_timer         = new Timer(debuggerHandle);
//        this->_timer->OnTime = []() {
//            // std::cout << "on time" << std::endl;
//            for(auto &item : debugger->_memberMap) {
//                std::cout << item.first << " : " << item.second << std::endl;
//            }
//        };
//        this->_isActive = true;
//        this->_timer->Start(timeout, timeout);
//
//        // debuggerHandle->Start();
//        // debuggerHandle->Detach();
//        return true;
//    };
//    inline bool IsActive() const { return this->_isActive; }
//    void        Destroy() {
//        //    debuggerHandle->Join();
//        delete this->_timer;
//        this->_timer = nullptr;
//    };
//
//    void MemberAdd(const std::string &name) { _memberMap[name]++; };
//    void MemberRemove(const std::string &name) { _memberMap[name]--; };
//
//    Timer *_timer{nullptr};
//    bool   _isActive{false};
//    //
//    std::unordered_map<std::string, std::atomic<int64_t>> _memberMap;
//};
//
// inline Debugger *debugger = new Debugger;
// #ifndef NDEBUG
//
// #define DEBUGADD(name) \
//    if(debugger != nullptr && debugger->IsActive()) \
//        debugger->MemberAdd(name);
//
// #define DEBUGRM(name) \
//    if(debugger != nullptr && debugger->IsActive()) \
//        debugger->MemberRemove(name);
//
// #define NEWADD DEBUGADD("new")
// #define DELADD \
//    DEBUGADD("delete") \ DEBUGRM("new")
//
// #define NEWARRAYADD DEBUGADD("new[]")
// #define DELARRAYADD \
//    DEBUGADD("delete[]") \ DEBUGRM("new[]")
//
// #else
// #define DEBUGADD(name) ;
// #define DEBUGRM(name) ;
//
// #define NEWADD DEBUGADD("new");
// #define DELADD ;
//
// #define NEWARRAYADD DEBUGADD("new[]");
// #define DELARRAYADD ;
//
// #endif
//
// class WTimeDebugger {
// public:
//    WTimeDebugger() { this->_stamp = this->GetTime(); }
//
//    void tick() {
//        int64_t now  = this->GetTime();
//        this->_stamp = now;
//    }
//
// private:
//    int64_t GetTime() {
//        std::chrono::microseconds ms = std::chrono::duration_cast<std::chrono::microseconds>(
//                std::chrono::system_clock::now().time_since_epoch());
//        return ms.count();
//    };
//
// private:
//    int64_t _stamp;
//};
//
//[[maybe_unused]] extern WTimeDebugger time_debug;

} // namespace wutils::debug

#endif // UTILS_DEBUGGER_H
