#include <vector>
#include <set>
#include <map>
// #include <list>
#include <thread>
#include "../WOS.h"
#include "WBaseSession.hpp"
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

public:
    explicit WSingleTcpServer() {};
    WSingleTcpServer(const WSingleTcpServer& other) = delete;
    WSingleTcpServer& operator=(const WSingleTcpServer& other) = delete;
    virtual ~WSingleTcpServer() {};

    // class life time
    bool Init();
    void Close();
    void Destroy();
    // thread lifetime
    void run();
    void WaitForQuit();

    // class methods
    bool AddAccepter(const std::string & IpAddress, uint16_t port);

    const uint16_t GetActiveConnectionCount() { return this->_connectionList.size(); };
    const uint16_t GetTempConnectionCount() { return this->_connectionTemp.size(); };

protected:
    // override listener methods
    bool OnNewConnection(base_socket_type socket, const WEndPointInfo& peerInfo) override;

    void OnNewSession(SessionNode* node) override;
    void OnSessionClosed(SessionNode* node) override;

private:
    void Loop();

    bool UpdateConnectionTemp();


private:
    std::thread* _workThread{nullptr};
    bool _running{false};

    WNetWorkHandler* _handler;
    std::map<base_socket_type, WNetAccepter*> _accepterMap;
    // 内存池设计
    SessionList _connectionList;
    SessionList _connectionTemp;
    const int connectionsIncrease = 100;    // 内存池增长

};










}   // namespace wlb::NetWork

#endif //

