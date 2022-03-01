#include "WNetWork/WMultiTcpServer.hpp"
#include <iostream>
#include "Logger.h"

namespace wlb::NetWork
{

using namespace wlb::Log;

/////////////////////////////////
// timer

WTimerHandler* timeHandler = new WTimerEpoll;










////////////////////////////////////////////////////////////////
// server

bool WMultiTcpServer::Init(uint16_t threads, const WSessionStyle& style)
{
    this->_threadsCount = threads < 1 ? 1 : threads;
    this->_servers.clear();
    
    for (size_t i = 0; i < this->_threadsCount; ++i)
    {
        WSingleTcpServer* the = new(std::nothrow) WSingleTcpServer(this);
        if ( the == nullptr || !the->Init(style))
        {
            return false;
        }
        
        this->_servers.push_back(the);
    }

    if (!timeHandler->Init())
    {
        return false;
    }

    this->_timer = new(std::nothrow) WServerTimer(this);
    if (this->_timer == nullptr)
    {
        return false;
    }
    
    this->_timer->Start(1000, 1000);

    return true;
}

void WMultiTcpServer::Close()
{
    this->_isRunning = false;
    timeHandler->Close();

    // 采用迭代器，防止init失败时，this->_threadsCount失效
    for (auto it : this->_servers)
    {
        it->Close();
    }
}

void WMultiTcpServer::Destroy()
{
    // 采用迭代器，防止init失败时，this->_threadsCount失效
    for (auto it : this->_servers)
    {
        it->Destroy();
        delete it;
    }
    this->_servers.clear();
    if (this->_timerThread != nullptr)
    {
        delete this->_timerThread;
        this->_timerThread = nullptr;
    }
    
    
    this->_listener = nullptr;

    delete this->_timer;

    timeHandler->Destroy();
}

void WMultiTcpServer::run()
{
    this->_isRunning = true;

    for (auto it : this->_servers)
    {
        it->run();
    }

    this->_timerThread = new std::thread(&WMultiTcpServer::Loop, this);
}

void WMultiTcpServer::WaitForQuit()
{
    for (auto it : this->_servers)
    {
        it->WaitForQuit();
    }
}

bool WMultiTcpServer::AddAccepter(const std::string & IpAddress, uint16_t port)
{
    for (auto it : this->_servers)
    {
        if (!it->AddAccepter(IpAddress, port))
        {
            return false;
        }
    }
    
    return true;
}


bool WMultiTcpServer::OnConnected(WBaseSession::SessionId id, const WEndPointInfo& peerInfo)
{
    if (this->_listener == nullptr)
    {
        return false;
    }
    return this->_listener->OnConnected(id, peerInfo);
}
bool WMultiTcpServer::OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message)
{
    if (this->_listener == nullptr)
    {
        return false;
    }

    return this->_listener->OnSessionMessage(id, recieve_message, send_message);
}
bool WMultiTcpServer::OnSessionClosed(WBaseSession::SessionId id)
{
    if (this->_listener == nullptr)
    {
        return false;
    }
    
    return this->_listener->OnSessionClosed(id);
}

bool WMultiTcpServer::OnSessionShutdown(WBaseSession::SessionId id) 
{
    if (this->_listener == nullptr)
    {
        return false;
    }
    
    return this->_listener->OnSessionShutdown(id);
}
bool WMultiTcpServer::OnSessionError(WBaseSession::SessionId id, int error_code) 
{
    if (this->_listener == nullptr)
    {
        return false;
    }
    
    return this->_listener->OnSessionError(id, error_code);
}

void WMultiTcpServer::Loop()
{
    while (this->_isRunning)
    {
        timeHandler->GetAndEmitTimer(-1);
    }
}

void WMultiTcpServer::OnTime(timerfd id)
{
    if (this->_timer->GetId() == id)
    {
        int index = 0;
        for (auto it : this->_servers)
        {
            LOG(L_INFO) << index++ << " " << it->GetActiveSessionCount() << " " << it->GetTempSessionCount();
        }
        
    }
}


}
