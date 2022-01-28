#include <vector>
#include <map>
#include <thread>
#include "WOS.h"
#include "WSession.hpp"


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
    bool OnError(base_socket_type socket, std::string error) override;
    bool OnClosed(base_socket_type socket) override;
    bool OnShutdown(base_socket_type socket) override;
    bool OnRead(base_socket_type socket) override;
    bool OnWrite(base_socket_type socket) override;

private:
    void Loop();

    void RemoveSession(std::map<base_socket_type, WBaseSession *>::iterator it);

private:
    std::thread* _workThread{nullptr};
    bool _running{false};

    WNetWorkHandler* _handler;
    std::map<base_socket_type, WBaseSession*> _sessionMap;
    std::map<base_socket_type, WNetAccepter*> _accepterMap;
};










}   // namespace wlb::NetWork

#endif //

