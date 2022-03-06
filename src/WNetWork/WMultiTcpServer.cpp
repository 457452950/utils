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

bool WMultiTcpServer::Init(uint16_t threads)
{
    this->_threadsCount = threads < 1 ? 1 : threads;
    this->_servers.clear();
    
    for (size_t i = 0; i < this->_threadsCount; ++i)
    {
        WSingleTcpServer* the = new(std::nothrow) WSingleTcpServer();
        if ( the == nullptr || !the->Init())
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
    
    // this->_timer->Start(1000, 1000);


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
            LOG(L_INFO) << index++ << " " << it->GetActiveConnectionCount() << " " << it->GetTempConnectionCount();
            std::cout << index++ << " " << it->GetActiveConnectionCount() << " " << it->GetTempConnectionCount() << std::endl;
        }
        
    }
}


}
