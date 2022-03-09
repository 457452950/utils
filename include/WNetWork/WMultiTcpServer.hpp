#pragma once

#include <thread>
#include <list>
#include "WSingleTcpServer.hpp"
#include "../WTimer.hpp"


namespace wlb::NetWork
{

/////////////////////////////////////////////////////////
// timer
extern WTimerHandler* timeHandler;

class WServerTimer : public wlb::WTimer
{
public:
    explicit WServerTimer(WTimerHandler::Listener* listener) 
            : WTimer(listener, timeHandler) {}
    virtual ~WServerTimer() {}

};




////////////////////////////////////////////////////////////////////////
// server

class WMultiTcpServer : public WTimerHandler::Listener
{
public:
    explicit WMultiTcpServer() {};
    WMultiTcpServer(const WMultiTcpServer& other) = delete;
    WMultiTcpServer& operator=(const WMultiTcpServer& other) = delete;
    virtual ~WMultiTcpServer() {};

    // class lifetime
    bool Init(uint16_t threads);
    void Close();
    void Destroy();

    // thread lifetime
    void run();
    void WaitForQuit();

    // class methods
    bool AddAccepter(const std::string & IpAddress, uint16_t port);

private:
    void Loop();

protected:
    
    // timer override
    virtual void OnTime(WTimer* timer) override;
private:

    // threads
    uint16_t _threadsCount{0};
    std::list<WSingleTcpServer*> _servers;

    bool _isRunning{false};
    std::thread* _timerThread{nullptr};

    WServerTimer* _timer{nullptr};
};


}
