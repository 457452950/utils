#include "WNetWork/WSingleTcpServer.hpp"

#include <iostream>

namespace wlb::NetWork
{

bool WSingleTcpServer::Init(WNetWorkHandler* handler)
{
    this->_handler = handler;
    if ( !this->_handler->Init(100))
    {
        std::cout << "WSingleTcpServer::Init(): Failed to initialize" << std::endl;
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
    WNetAccepter* acc = new(std::nothrow) WNetAccepter();
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

bool WSingleTcpServer::OnError(base_socket_type socket, int error_code)
{
    auto it = this->_sessionMap.find(socket);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    if ( !it->second->OnError(socket, error_code) )
    {
        RemoveSession(it);
        return false;
    }
    return true;
}

bool WSingleTcpServer::OnClosed(base_socket_type socket)
{
    auto it = this->_sessionMap.find(socket);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    if ( !it->second->OnClosed(socket) )
    {
        RemoveSession(it);
        return false;
    }
    return true;
}
bool WSingleTcpServer::OnShutdown(base_socket_type socket)
{
    auto it = this->_sessionMap.find(socket);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    if ( !it->second->OnShutdown(socket) )
    {
        RemoveSession(it);
        return false;
    }
    return true;
}
bool WSingleTcpServer::OnRead(base_socket_type socket)
{
    std::cout << "WSingleTcpServer::OnRead " << std::endl;

    // receive
    auto it = this->_sessionMap.find(socket);
    if (it != this->_sessionMap.end())
    {
        if ( !it->second->OnRead(socket) )
        {
            RemoveSession(it);
            return false;
        }
        return true;
    }

    // accept connection
    std::cout << "WSingleTcpServer::OnRead() not session " << std::endl;
    auto accept = this->_accepterMap.find(socket);
    if (accept == this->_accepterMap.end())
    {
        // cant find accept socket 
        return false;
    }
    
    WPeerInfo info;
    std::cout << "WSingleTcpServer::OnRead() has session " << _sessionMap.size() << std::endl;
    base_socket_type cli_sock = accept->second->Accept(info);
    if (cli_sock <= 0) 
    {
        // error
        return false;
    }

    if (this->_service._OnConnected)
    {
        if ( !(this->_service._OnConnected)(socket, info) )
        {
            // refuse connection
            ::close(cli_sock);
            return true;
        }
    }
    
        
    if ( !this->CreateNewSession(cli_sock, info) )
    {
        return false;
    }
        
    return true;
}
bool WSingleTcpServer::OnWrite(base_socket_type socket)
{
    auto it = this->_sessionMap.find(socket);
    if  (it == this->_sessionMap.end())
    {
        return false;
    }

    if ( !it->second->OnWrite(socket) )
    {
        RemoveSession(it);
        return false;
    }
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

bool WSingleTcpServer::CreateNewSession(base_socket_type socket, const WPeerInfo& peerInfo)
{
    WBaseSession* session = new(std::nothrow) WFloatBufferSession(this->_service);
    if (session == nullptr)
    {
        return false;
    }
    
    if ( !session->Init(this->_handler, 102400, 4) )
    {
        session->Destroy();
        delete session;
        return false;
    }
    if ( !session->SetSocket(socket, peerInfo) )
    {
        session->Destroy();
        delete session;
        return false;
    }
    this->_sessionMap.insert(std::make_pair(socket, session));
    return true;
}

}
