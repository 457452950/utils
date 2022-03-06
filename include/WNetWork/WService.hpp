#pragma once
#include <string>
#include <functional>
#include "WNetWorkUtils.h"

namespace wlb::NetWork
{

class WService
{
public:
    using ConnectionId = base_socket_type;

    // returns false if the connection need to close 
    using WOnConnectedHandler = std::function<bool (ConnectionId connectionId, const WEndPointInfo& peer)>;
    using WOnMessageHanler = std::function<bool (ConnectionId connectionId, const std::string& receive_message, std::string& send_message)>;
    using WOnErrorHandler = std::function<bool (ConnectionId connectionId, const std::string& error_message)>;
    using WOnShutdownHandler = std::function<void (ConnectionId connectionId)>;
    using WOnDisonnectedHandler = std::function<void (ConnectionId connectionId)>;
public:
    WService() = default;
    ~WService() = default;

    WOnConnectedHandler _OnConnected;
    WOnMessageHanler _OnMessage;
    WOnErrorHandler _OnError;
    WOnShutdownHandler _OnShutDown;
    WOnDisonnectedHandler _OnDisconnected;
};







}
