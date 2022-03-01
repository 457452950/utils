#pragma once
#include <string>
#include <functional>
#include "WNetWorkUtils.h"

namespace wlb::NetWork
{

class WService
{
public:
    using SessionId = base_socket_type;

    // returns false if the connection need to close 
    using WOnConnectedHandler = std::function<bool (SessionId sessionId, const WEndPointInfo& peer)>;
    using WOnMessageHanler = std::function<bool (SessionId sessionId, const std::string& receive_message, std::string& send_message)>;
    using WOnErrorHandler = std::function<bool (SessionId sessionId, const std::string& error_message)>;
    using WOnShutdownHandler = std::function<void (SessionId sessionId)>;
    using WOnDisonnectedHandler = std::function<void (SessionId sessionId)>;
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
