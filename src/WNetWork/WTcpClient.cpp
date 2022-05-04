#include "WNetWork/WTcpClient.hpp"

namespace wlb::NetWork
{


bool WTcpClient::Init()
{
    this->_socket = MakeTcpV4Socket();
    if (this->_socket == -1)
    {
        return false;
    }
    return true;
}
void WTcpClient::Destroy()
{
    if (this->_socket != -1)
    {
        ::close(this->_socket);
        this->_socket = -1;
    }
    
}

bool WTcpClient::Send(const std::string& message)
{
    return this->_connection->Send(message);
}
void WTcpClient::CloseConnection()
{
    this->_connection->CloseConnection();
}
bool WTcpClient::ConnectToHost(const WEndPointInfo& host)
{
    this->_serverInfo = host;
    if (!wlb::NetWork::ConnectToHost(this->_socket, this->_serverInfo))
    {
        return false;
    }
    if (!this->_connection->SetConnectedSocket(this->_socket, this->_serverInfo))
    {
        return false;
    }
    return true;
}
bool WTcpClient::ConnectToHost(const std::string& address, uint16_t port)
{
    this->_serverInfo = {address, port};
    if (!wlb::NetWork::ConnectToHost(this->_socket, this->_serverInfo))
    {
        return false;
    }
    if (this->_connection->SetConnectedSocket(this->_socket, this->_serverInfo))
    {
        return false;
    }
    return true;
}








}



