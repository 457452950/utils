#include "WNetWork/WSingleTcpServer.hpp"

#include <iostream>

namespace wlb::NetWork
{

bool WSingleTcpServer::Init(const WSessionStyle& style)
{
    this->_sessionTemp.Init(); 
    this->_sessionList.Init();
    this->_accepterMap.clear();

    this->_sessionStyle = style;

    this->_handler = new(std::nothrow) WEpoll();
    if ( this->_handler != nullptr && !this->_handler->Init(100))
    {
        return false;
    }

    if (!this->UpdateSessionTemp())
    {
        return false;
    }
    
    
    return true;
}

void WSingleTcpServer::Close()
{
    this->_running = false;

    this->_sessionTemp.clear();
    this->_sessionList.clear();

    this->_handler->Close();
}

void WSingleTcpServer::Destroy()
{
    if (this->_workThread != nullptr)
    {
        delete this->_workThread;
        this->_workThread = nullptr;
    }

    this->_sessionTemp.clear();
    this->_sessionList.clear();

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

bool WSingleTcpServer::OnSessionError(Node* node)
{
    if  (!this->_sessionList.has_node(node))
    {
        this->_sessionTemp.strong_push_back(node);
        return false;
    }

    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionError(node->val);
    }
    
    // 回收内存池
    this->_sessionList.erase(node);
    this->_sessionTemp.push_back(node);

    return true;
}

bool WSingleTcpServer::OnSessionClosed(Node* node)
{
    if (!this->_sessionList.has_node(node))
    {
        this->_sessionTemp.strong_push_back(node);
        return false;
    }
    
    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionClosed(node->val);
    }

    // 回收内存池
    this->_sessionList.erase(node);
    this->_sessionTemp.push_back(node);
    
    return true;
}
bool WSingleTcpServer::OnSessionShutdown(Node* node)
{
    if (!this->_sessionList.has_node(node))
    {
        this->_sessionTemp.strong_push_back(node);
        return false;
    }
    

    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionShutdown(node->val);
    }
    

    return true;
}
bool WSingleTcpServer::OnSessionMessage(Node* node, const std::string& recieve_message)
{
    

    if ( !this->_listener->OnSessionMessage(node->val, recieve_message))
    {
        return false;
    }

    return true;
}
bool WSingleTcpServer::OnConnected(base_socket_type socket, const WEndPointInfo& peerInfo)
{
    if (this->_sessionTemp.empty() && !this->UpdateSessionTemp())
    {
        // cant new session
        std::cout << "cant new session" << std::endl;
        exit(-1);
        return true;
    }
    
    auto node = this->_sessionTemp.front();
    WBaseSession* session = node->val;
    if (session->isConnected())
    {
        std::cout << "session is connected" << std::endl;
        exit(-2);
    }
    
    if ( !session->SetConnectedSocket(socket, peerInfo) )
    {
        return false;
    }
    
    if (this->_listener->OnConnected(session, peerInfo))
    {
        this->_sessionTemp.pop_front();
        this->_sessionList.push_back(node);
    }

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

void WSingleTcpServer::RemoveSession(Node* node)
{

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

bool WSingleTcpServer::UpdateSessionTemp()
{
    for (size_t index = 0; index < this->sessionsIncrease; ++index)
    {
        WBaseSession* session = nullptr;
        Node* node = new(std::nothrow) Node;
        if (this->_sessionStyle.type == WSessionType::WFloatSessions)
        {
            session = new(std::nothrow) WFloatBufferSession(this, node);
        }
        else
        {
            session = new(std::nothrow) WFixedBufferSession(this, node);
        }
        
        if (node == nullptr || session == nullptr)
        {
            return false;
        }
        node->val = session;
        
        if ( !session->Init(this->_handler, this->_sessionStyle.maxBufferSize, this->_sessionStyle.flag) )
        {
            return false;
        }
        
        
        this->_sessionTemp.push_front(node);
    }
    return true;
}

}
