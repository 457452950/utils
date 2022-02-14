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

class WMultiTcpServer : public WSingleTcpServer::Listener,
                        public WTimerHandler::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual bool OnConnected(WBaseSession::SessionId id, const WPeerInfo& peerInfo) = 0;
        virtual bool OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message) = 0;
        virtual bool OnSessionClosed(WBaseSession::SessionId id) = 0;
        // virtual bool OnSessionShutdown(WBaseSession::SessionId id) = 0;
        // virtual bool OnSessionError(WBaseSession::SessionId id, int error_code) = 0;
    };
public:
    explicit WMultiTcpServer(Listener* listener) : _listener(listener) {};
    WMultiTcpServer(const WMultiTcpServer& other) = delete;
    WMultiTcpServer& operator=(const WMultiTcpServer& other) = delete;
    virtual ~WMultiTcpServer() {};

    // class lifetime
    bool Init(uint16_t threads, const WSessionStyle& style);
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
    // overrides
    virtual bool OnConnected(WBaseSession::SessionId id, const WPeerInfo& peerInfo) override;
    virtual bool OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message) override;
    virtual bool OnSessionClosed(WBaseSession::SessionId id) override;
    // virtual bool OnSessionShutdown(WBaseSession::SessionId id) override;
    // virtual bool OnSessionError(WBaseSession::SessionId id, int error_code) override;
    
    // timer override
    virtual void OnTime(timerfd id) override;
private:
    Listener* _listener{nullptr};

    // threads
    uint16_t _threadsCount{0};
    std::list<WSingleTcpServer*> _servers;

    bool _isRunning{false};
    std::thread* _timerThread{nullptr};

    WServerTimer* _timer{nullptr};
};


}