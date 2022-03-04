#include <vector>
#include <set>
#include <map>
// #include <list>
#include <thread>
#include "../WOS.h"
#include "WSession.hpp"
#include "WService.hpp"
#include "WList.hpp"


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
        virtual bool OnConnected(WSession* session, const WEndPointInfo& peerInfo) = 0;
        virtual bool OnSessionMessage(WSession* session, const std::string& recieve_message) = 0;
        virtual bool OnSessionClosed(WSession* session) = 0;
        virtual bool OnSessionShutdown(WSession* session) = 0;
        virtual bool OnSessionError(WSession* session) = 0;
    };
public:
    explicit WSingleTcpServer(Listener* listener) : _listener(listener){};
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

    const uint16_t GetActiveSessionCount() { return this->_sessionList.size(); };
    const uint16_t GetTempSessionCount() { return this->_sessionTemp.size(); };

protected:
    // override listener methods
    bool OnConnected(base_socket_type socket, const WEndPointInfo& peerInfo) override;

    bool OnSessionMessage(Node* node, const std::string& recieve_message) override;
    bool OnSessionClosed(Node* node) override;
    bool OnSessionShutdown(Node* node) override;
    bool OnSessionError(Node* node) override;


private:
    void Loop();

    bool UpdateSessionTemp();

    // bool CreateNewSession(base_socket_type socket, const WEndPointInfo& peerInfo);
    void RemoveSession(Node* node);

private:
    std::thread* _workThread{nullptr};
    bool _running{false};

    WNetWorkHandler* _handler;
    std::map<base_socket_type, WNetAccepter*> _accepterMap;
    // 内存池设计
    List _sessionList;
    List _sessionTemp;
    const int sessionsIncrease = 50;    // 内存池增长

    // session style
    WSessionStyle _sessionStyle;

    // service handler
    Listener* _listener;
public:
};










}   // namespace wlb::NetWork

#endif //

