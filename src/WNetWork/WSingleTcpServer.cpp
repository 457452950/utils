#include "WNetWork/WSingleTcpServer.hpp"

#include <iostream>

namespace wlb::NetWork
{

bool WSingleTcpServer::Init(const WSessionStyle& style)
{
    this->_sessionTemp.clear(); 
    this->_sessionMap.clear();
    this->_accepterMap.clear();

    this->_sessionStyle = style;

    this->_handler = new(std::nothrow) WEpoll();
    if ( this->_handler != nullptr && !this->_handler->Init(100))
    {
        return false;
    }

    if (!this->UpdateSesssionTemp())
    {
        return false;
    }
    
    
    return true;
}

void WSingleTcpServer::Close()
{
    this->_running = false;

    for (auto it : this->_sessionMap)
    {
        it.second->Close();
    }

    this->_handler->Close();
}

void WSingleTcpServer::Destroy()
{
    if (this->_workThread != nullptr)
    {
        delete this->_workThread;
        this->_workThread = nullptr;
    }

    while (!this->_sessionTemp.empty())
    {
        auto it = this->_sessionTemp.front();
        this->_sessionTemp.pop_front();
        delete it;
    }
    for (auto it : this->_sessionMap)
    {
        it.second->Destroy();
        delete it.second;
    }
    this->_sessionMap.clear();

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

bool WSingleTcpServer::OnSessionError(WBaseSession::SessionId id, int error_code)
{
    auto it = this->_sessionMap.find(id);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionError(id, error_code);
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

    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionClosed(id);
    }

    // 回收内存池
    this->_sessionTemp.push_back(it->second);
    this->_sessionMap.erase(it);
    
    return true;
}
bool WSingleTcpServer::OnSessionShutdown(WBaseSession::SessionId id)
{
    auto it = this->_sessionMap.find(id);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionShutdown(id);
    }
    

    return true;
}
bool WSingleTcpServer::OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message)
{
    

    if ( !this->_listener->OnSessionMessage(id, recieve_message, send_message))
    {
        return false;
    }

    return true;
}
bool WSingleTcpServer::OnConnected(base_socket_type socket, const WEndPointInfo& peerInfo)
{
    if (this->_listener != nullptr)
    {
        if ( !this->_listener->OnConnected(socket, peerInfo))
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
    
    if ( !session->SetConnectedSocket(socket, peerInfo) )
    {
        return false;
    }
    

    this->_sessionMap.insert(std::make_pair(socket, session));
    this->_sessionTemp.pop_front();
    return true;
}

void WSingleTcpServer::Loop()
{
    while (_running)
    {
        this->_handler->GetAndEmitEvents(-1);
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

// bool WSingleTcpServer::CreateNewSession(base_socket_type socket, const WEndPointInfo& peerInfo)
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
        WBaseSession* session = nullptr;
        if (this->_sessionStyle.type == WSessionType::WFloatSessions)
        {
            session = new(std::nothrow) WFloatBufferSession(this);
        }
        else
        {
            session = new(std::nothrow) WFixedBufferSession(this);
        }
        
        if (session == nullptr)
        {
            return false;
        }
        
        
        if ( !session->Init(this->_handler, this->_sessionStyle.maxBufferSize, this->_sessionStyle.flag) )
        {
            return false;
        }
        
        
        this->_sessionTemp.push_front(session);
    }
    return true;
}

}
