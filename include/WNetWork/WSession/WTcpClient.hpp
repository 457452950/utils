#pragma once
#include "WConnection.hpp"
#include "WEpoll.hpp"

namespace wlb::NetWork
{

class WTcpClient : public WBaseConnection::Listener
{
public:
    WTcpClient(WNetWorkHandler* handler) : _handler(handler) {};
    ~WTcpClient() { };

    virtual bool Init();
    virtual void Destroy();

    virtual bool Send(const std::string& message);
    virtual void CloseConnection();
    virtual bool ConnectToHost(const WEndPointInfo& host);
    virtual bool ConnectToHost(const std::string& address, uint16_t port);

public:
    // override
    virtual void OnConnectionMessage(const std::string& recieve_message) = 0;
    virtual void OnConnectionClosed() = 0;
    virtual void OnConnectionShutdown() = 0;
    virtual void OnConnectionError() = 0;

protected:
    WNetWorkHandler* _handler{nullptr};
    WBaseConnection* _connection{nullptr};
    WEndPointInfo _serverInfo;
    base_socket_type _socket{-1};
};










}

