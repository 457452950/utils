#include <vector>
#include <map>
#include <thread>
#include "WOS.h"
#include "WSession.hpp"
#include "WService.hpp"


#if OS_IS_LINUX

namespace wlb::NetWork
{

class WSingleTcpServer : public WNetWorkHandler::Listener
{
public:
    explicit WSingleTcpServer() = default;
    WSingleTcpServer(const WSingleTcpServer& other) = delete;
    WSingleTcpServer& operator=(const WSingleTcpServer& other) = delete;
    ~WSingleTcpServer() {};

    // class life time
    bool Init(WNetWorkHandler* handler);
    void Close();
    void Destroy();
    // thread lifetime
    void run();
    void WaitForQuit();

    // class methods
    bool AddAccepter(const std::string & IpAddress, uint16_t port);

    // override Listener
    bool OnError(base_socket_type socket, int error_code) override;
    bool OnClosed(base_socket_type socket) override;
    bool OnShutdown(base_socket_type socket) override;
    bool OnRead(base_socket_type socket) override;
    bool OnWrite(base_socket_type socket) override;

private:
    void Loop();

    bool CreateNewSession(base_socket_type socket, const WPeerInfo& peerInfo);
    void RemoveSession(std::map<base_socket_type, WBaseSession *>::iterator it);

private:
    std::thread* _workThread{nullptr};
    bool _running{false};

    WNetWorkHandler* _handler;
    std::map<base_socket_type, WBaseSession*> _sessionMap;
    std::map<base_socket_type, WNetAccepter*> _accepterMap;

    // service handler
    WService _service;
public:
    inline void RegisterOnConnectedHandler(WService::WOnConnectedHandler callback) {this->_service._OnConnected = callback;};
    inline void RegisterOnMessageHandler(WService::WOnMessageHanler callback) {this->_service._OnMessage = callback;};
    inline void RegisterOnErrorHandler(WService::WOnErrorHandler callback) {this->_service._OnError = callback;};
    inline void RegisterOnShutdownHandler(WService::WOnShutdownHandler callback) {this->_service._OnShutDown = callback;};
    inline void RegisterOnDisconnectHandler(WService::WOnDisonnectedHandler callback) {this->_service._OnDisconnected = callback;};
};










}   // namespace wlb::NetWork

#endif //

