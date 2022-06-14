#include <map>
#include <set>
#include <vector>
// #include <list>
#include <thread>
#include "../../WList.hpp"
#include "../../WOS.h"
#include "WBaseSession.hpp"

#if OS_IS_LINUX

namespace wlb::network {

class WSingleTcpServer : public WNetAccepter::Listener, public WBaseSession::Listener {
public:
public:
    explicit WSingleTcpServer() = default;
    ~WSingleTcpServer() override;
    // no copyable
    WSingleTcpServer(const WSingleTcpServer &other)            = delete;
    WSingleTcpServer &operator=(const WSingleTcpServer &other) = delete;

    // class lifetime
    bool Init();
    void Close();
    void Destroy();
    // thread lifetime
    void run();
    void WaitForQuit();

    // class methods
    bool AddAccepter(const std::string &IpAddress, uint16_t port);
    bool AddAccepter(const WEndPointInfo &local_info);

    inline uint16_t GetActiveConnectionCount() { return this->session_list_.size(); };
    inline uint16_t GetTempConnectionCount() { return this->session_temp_.size(); };

protected:
    // override listener methods
    bool OnNewConnection(base_socket_type socket, const WEndPointInfo &peerInfo) override;

    void OnNewSession(SessionNode *node) override;
    void OnSessionClosed(SessionNode *node) override;

private:
    void Loop();
    bool IncreaseConnectionTemp();

private:
    std::thread *work_thread_{nullptr};
    bool         running_{false};
    //
    WNetWorkHandler                           *handler_{nullptr};
    std::map<base_socket_type, WNetAccepter *> accepter_map_;
    // 内存池设计
    SessionList    session_list_;
    SessionList    session_temp_;
    const uint16_t connections_increase_ = 100; // 内存池增长
};

} // namespace wlb::network

#endif //
