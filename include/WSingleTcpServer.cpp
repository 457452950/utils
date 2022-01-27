#include "WSingleTcpServer.hpp"

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
        return false;
    }
    std::cout << "acc Init succ" << std::endl;

    _accepterMap.insert(std::make_pair(acc->GetListenSocket(), acc));
    return true;
}

bool WSingleTcpServer::OnError(base_socket_type socket, std::string error)
{
    auto it = this->_sessionMap.find(socket);

    if (it != this->_sessionMap.end())
    {
        if ( !it->second->OnError(socket, error) )
        {
            this->_handler->RemoveSocket(socket);
            it->second->Close();
        }
    }
    
}

bool WSingleTcpServer::OnClosed(base_socket_type socket)
{
    auto it = this->_sessionMap.find(socket);

    if (it != this->_sessionMap.end())
    {
        if ( !it->second->OnClosed(socket) )
        {
            this->_handler->RemoveSocket(socket);
            it->second->Close();
        }
    }
}
bool WSingleTcpServer::OnRead(base_socket_type socket)
{
    auto it = this->_sessionMap.find(socket);

    if (it != this->_sessionMap.end())
    {
        if ( !it->second->OnRead(socket) )
        {
            this->_handler->RemoveSocket(socket);
            it->second->Close();
        }
    }
    else
    {
        auto accept = this->_accepterMap.find(socket);
        base_socket_type cli_sock = accept->second->Accept();

        WBaseSession* session = new WFloatBufferSession();
        session->Init(this->_handler, 100, 2);
        session->SetSocket(cli_sock);
        this->_sessionMap.insert(std::make_pair(cli_sock, session));
    }
    
}
bool WSingleTcpServer::OnWrite(base_socket_type socket)
{
    auto it = this->_sessionMap.find(socket);

    if (it != this->_sessionMap.end())
    {
        if ( !it->second->OnWrite(socket) )
        {
            this->_handler->RemoveSocket(socket);
            it->second->Close();
        }
    }
}

void WSingleTcpServer::Loop()
{
    while (_running)
    {
        this->_handler->GetAndEmitEvents();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

}
