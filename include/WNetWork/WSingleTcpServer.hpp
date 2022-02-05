#include <vector>
#include <map>
#include <list>
#include <thread>
#include "WOS.h"
#include "WSession.hpp"
#include "WService.hpp"


#if OS_IS_LINUX

namespace wlb::NetWork
{

class WSingleTcpServer : 
                        public WNetAccepter::Listener, 
                        public WBaseSession::Listener
{
public:
    class Listener 
    {
    public:
        virtual ~Listener() {}
        virtual bool OnConnected(WBaseSession::SessionId id, const WPeerInfo& peerInfo) = 0;
        virtual bool OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message) = 0;
        virtual bool OnSessionClosed(WBaseSession::SessionId id) = 0;
        // virtual bool OnSessionShutdown(WBaseSession::SessionId id) = 0;
        // virtual bool OnSessionError(WBaseSession::SessionId id, int error_code) = 0;

    };
public:
    explicit WSingleTcpServer(Listener* listener) : _service(listener){};
    WSingleTcpServer(const WSingleTcpServer& other) = delete;
    WSingleTcpServer& operator=(const WSingleTcpServer& other) = delete;
    virtual ~WSingleTcpServer() {};

    // class life time
    bool Init(const WSessionStyle& style);
    void Close();
    void Destroy();
    // thread lifetime
    void run();
    void WaitForQuit();

    // class methods
    bool AddAccepter(const std::string & IpAddress, uint16_t port);

protected:
    // override listener methods
    bool OnConnected(base_socket_type socket, const WPeerInfo& peerInfo) override;

    bool OnSessionMessage(WBaseSession::SessionId id, const std::string& recieve_message, std::string& send_message) override;
    bool OnSessionClosed(WBaseSession::SessionId id) override;
    bool OnSessionShutdown(WBaseSession::SessionId id) override;
    bool OnSessionError(WBaseSession::SessionId id, int error_code) override;


private:
    void Loop();

    bool UpdateSesssionTemp();

    // bool CreateNewSession(base_socket_type socket, const WPeerInfo& peerInfo);
    void RemoveSession(std::map<base_socket_type, WBaseSession *>::iterator it);

private:
    std::thread* _workThread{nullptr};
    bool _running{false};

    WNetWorkHandler* _handler;
    std::map<base_socket_type, WNetAccepter*> _accepterMap;
    // 内存池设计
    std::map<WBaseSession::SessionId, WBaseSession*> _sessionMap;
    std::list<WBaseSession*> _sessionTemp;
    const int sessionsIncrease = 50;    // 内存池增长

    // session style
    WSessionStyle _sessionStyle;

    // service handler
    Listener* _service;
public:
};










}   // namespace wlb::NetWork

#endif //

