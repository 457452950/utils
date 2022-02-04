#include "WNetWork/WSingleTcpServer.hpp"

#include <iostream>

namespace wlb::NetWork
{

bool WSingleTcpServer::Init()
{
    this->_handler = new(std::nothrow) WEpoll();
    if ( this->_handler != nullptr && !this->_handler->Init(100))
    {
        std::cout << "WSingleTcpServer::Init(): Failed to initialize" << std::endl;
        return false;
    }

    if (!this->UpdateSesssionTemp())
    {
        return false;
    }
    
    std::cout << "WSingleTcpServer::Init(): succ" << std::endl;
    return true;
}

void WSingleTcpServer::Close()
{
    for (auto it : this->_sessionMap)
    {
        std::cout << "WSingleTcpServer::Close: " << it.first << std::endl;
        it.second->Close();
    }

    this->_handler->Close();
}

void WSingleTcpServer::Destroy()
{
    if (this->_workThread != nullptr)
    {
        delete this->_workThread;
    }

    while (!this->_sessionTemp.empty())
    {
        auto it = this->_sessionTemp.front();
        this->_sessionTemp.pop_front();
        delete it;
    }
    for (auto it : this->_sessionMap)
    {
        it.second->Close();
        it.second->Destroy();
        delete it.second;
        this->_sessionMap.erase(it.first);
    }
    for (auto it : this->_accepterMap)
    {
        delete it.second;
        this->_sessionMap.erase(it.first);
    }
    
    if (this->_handler != nullptr)
    {
        delete this->_handler;
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
        std::cout << "WNetAccepter new failed" << std::endl;
        return false;
    }
    std::cout << "WNetAccepter new succ" << std::endl;
    
    if ( !acc->Init(this->_handler, IpAddress, port) )
    {
        std::cout << "acc Init failed" << std::endl;
        delete acc;
        return false;
    }
    std::cout << "acc Init succ" << std::endl;

    _accepterMap.insert(std::make_pair(acc->GetListenSocket(), acc));
    return true;
}

bool WSingleTcpServer::OnSessionError(WBaseSession::SessionId id, int error_code)
{
    auto it = this->_sessionMap.find(id);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    return true;
}

bool WSingleTcpServer::OnSessionClosed(WBaseSession::SessionId id)
{
    auto it = this->_sessionMap.find(id);
    if  (it == this->_sessionMap.end())
    {
        // cant find the session
        return true;
    }

    if (this->_service != nullptr)
    {
        this->_service->OnSessionClosed(id);
    }

    // 回收内存池
    this->_sessionTemp.push_back(it->second);
    this->_sessionMap.erase(it);
    
    std::cout << "temp:" << this->_sessionTemp.size() << " session:" << this->_sessionMap.size() << std::endl;

    return true;
}
bool WSingleTcpServer::OnSessionShutdown(WBaseSession::SessionId id)
{
    auto it = this->_sessionMap.find(id);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    return true;
}
bool WSingleTcpServer::OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message)
{
    std::cout << "WSingleTcpServer::OnRead " << std::endl;

    if ( !this->_service->OnSessionMessage(id, recieve_message, send_message))
    {
        return false;
    }

    return true;
}
bool WSingleTcpServer::OnConnected(base_socket_type socket, const WPeerInfo& peerInfo)
{
    if (this->_service != nullptr)
    {
        if ( !this->_service->OnConnected(socket, peerInfo))
        {
            // 拒绝服务
            return false;
        }
    }
    
    if (this->_sessionTemp.empty() && !this->UpdateSesssionTemp())
    {
        // cant new session
        exit(-1);
        return true;
    }
    
    WBaseSession* session = this->_sessionTemp.front();
    if (session->isConnected())
    {
        exit(-2);
    }
    
    if ( !session->Init(this->_handler, 102400, 4) || !session->SetSocket(socket, peerInfo))
    {
        return false;
    }
    std::cout << "Session initialized ok!" << std::endl;

    this->_sessionMap.insert(std::make_pair(socket, session));
    this->_sessionTemp.pop_front();
    return true;
}

void WSingleTcpServer::Loop()
{
    while (_running)
    {
        this->_handler->GetAndEmitEvents();
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void WSingleTcpServer::RemoveSession(std::map<base_socket_type, WBaseSession *>::iterator it)
{
    this->_handler->RemoveSocket(it->first);
    it->second->Close();
    it->second->Destroy();
    delete it->second;
    this->_sessionMap.erase(it);
}

// bool WSingleTcpServer::CreateNewSession(base_socket_type socket, const WPeerInfo& peerInfo)
// {
//     WBaseSession* session = new(std::nothrow) WFloatBufferSession(this);
//     if (session == nullptr)
//     {
//         return false;
//     }
    
//     if ( !session->Init(this->_handler, 102400, 4) )
//     {
//         session->Destroy();
//         delete session;
//         return false;
//     }
//     if ( !session->SetSocket(socket, peerInfo) )
//     {
//         session->Destroy();
//         delete session;
//         return false;
//     }
//     this->_sessionMap.insert(std::make_pair(socket, session));
//     return true;
// }

bool WSingleTcpServer::UpdateSesssionTemp()
{
    for (size_t index = 0; index < this->sessionsIncrease; ++index)
    {
        WBaseSession* session = new(std::nothrow) WFloatBufferSession(this);
        if (session == nullptr)
        {
            return false;
        }
        
        this->_sessionTemp.push_front(session);
    }
    return true;
}

}
