#pragma once

#include <list>
#include <thread>
#include "../../WTimer.hpp"
#include "WSingleTcpServer.hpp"

namespace wlb::network {

/////////////////////////////////////////////////////////
// timer
extern WTimerHandler *timeHandler;

class WServerTimer : public wlb::WTimer {
public:
    explicit WServerTimer(WTimerHandler::Listener *listener) : WTimer(listener, timeHandler) {}
    ~WServerTimer() override = default;
};


////////////////////////////////////////////////////////////////////////
// server

class WMultiTcpServer : public WTimerHandler::Listener {
public:
    explicit WMultiTcpServer()  = default;
    ~WMultiTcpServer() override = default;
    // no copyable
    WMultiTcpServer(const WMultiTcpServer &other)            = delete;
    WMultiTcpServer &operator=(const WMultiTcpServer &other) = delete;

    // class lifetime
    bool Init(uint16_t threads);
    void Close();
    void Destroy();

    // thread lifetime
    void run();
    void WaitForQuit();

    // class methods
    bool AddAccepter(const std::string &IpAddress, uint16_t port);
    bool AddAccepter(const WEndPointInfo &local_info);

private:
    void Loop();

protected:
    // timer override
    void OnTime(WTimer *timer) override;

private:
    // threads
    uint16_t                      _threadsCount{0};
    std::list<WSingleTcpServer *> _servers;
    //
    bool         _isRunning{false};
    std::thread *_timerThread{nullptr};
    //
    WServerTimer *_timer{nullptr};
};

} // namespace wlb::network
