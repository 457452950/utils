#include "WNetWork/WSingleTcpServer.hpp"

#include <iostream>
#include <cassert>

namespace wlb::NetWork
{

bool WSingleTcpServer::Init()
{
    this->session_temp_.Init();
    this->session_list_.Init();
    this->_accepterMap.clear();

    this->_handler = CreateNetworkHandlerAndInit(128);
    if ( this->_handler == nullptr)
    {
        return false;
    }

    if (!this->UpdateConnectionTemp())
    {
        return false;
    }
    
    
    return true;
}

void WSingleTcpServer::Close()
{
    this->_running = false;

    this->session_temp_.clear();
    this->session_list_.clear();

    this->_handler->Close();
}

void WSingleTcpServer::Destroy()
{
    if (this->_workThread != nullptr)
    {
        delete this->_workThread;
        this->_workThread = nullptr;
    }

    this->session_temp_.clear();
    this->session_list_.clear();

    for (auto it : this->_accepterMap)
    {
        delete it.second;
    }
    this->_accepterMap.clear();
    
    if (this->_handler != nullptr)
    {
        delete this->_handler;
        this->_handler = nullptr;
    }
}

void WSingleTcpServer::run()
{
    this->_running = true;
    this->_workThread = new(std::nothrow) std::thread(&WSingleTcpServer::Loop, this);
}

void WSingleTcpServer::WaitForQuit()
{
    if (this->_workThread != nullptr && this->_workThread->joinable())
    {
        this->_workThread->join();
    }
}

bool WSingleTcpServer::AddAccepter(const std::string& IpAddress, uint16_t port)
{
    WNetAccepter* acc = new(std::nothrow) WNetAccepter(this);
    if (acc == nullptr)
    {
        
        return false;
    }
    
    
    if ( !acc->Init(this->_handler, IpAddress, port) )
    {
        
        delete acc;
        return false;
    }
    

    _accepterMap.insert(std::make_pair(acc->GetListenSocket(), acc));
    return true;
}

bool WSingleTcpServer::OnNewConnection(base_socket_type socket, const WEndPointInfo& peerInfo)
{
    if (this->session_temp_.empty() && !this->UpdateConnectionTemp())
    {
        // can't new connection
        std::cout << "cant new connection" << std::endl;
        exit(-1);
    }
    
    auto node = this->session_temp_.front();
    WBaseSession* session = node->val;
    if (session->IsConnected())
    {
        std::cout << "connection is connected" << std::endl;
        assert(session->IsConnected());
    }
    
    if ( !session->SetConnectedSocket(socket, peerInfo) )
    {
        session->Clear();
        return false;
    }
    
    return true;
}


void WSingleTcpServer::OnNewSession(SessionNode* node)
{
    this->session_temp_.erase(node);
    this->session_list_.push_back(node);
}
void WSingleTcpServer::OnSessionClosed(SessionNode* node)
{
    this->session_temp_.strong_push_back(node);
}

void WSingleTcpServer::Loop()
{
    while (_running)
    {
        this->_handler->GetAndEmitEvents(-1);
    }
}

bool WSingleTcpServer::UpdateConnectionTemp()
{
    for (size_t index = 0; index < this->connectionsIncrease; ++index)
    {
        auto* session = CreateNewSessionNodeAndInit(this, this->_handler);

        if (session == nullptr)
        {
            return false;
        }
        
        this->session_temp_.push_front(session);
    }
    return true;
}

}
