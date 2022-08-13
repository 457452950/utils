#pragma once
#include <cassert>
#include "WConnection.hpp"
#include "../WList.hpp"

namespace wlb::network {

class WBaseSession;

using SessionList = WList<WBaseSession *>;
using SessionNode = SessionList::WListNode;

class WBaseSession : public WBaseConnection::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void OnNewSession(SessionNode *session) = 0;
        virtual void OnSessionClosed(SessionNode *session) = 0;
    };

    explicit WBaseSession(WBaseSession::Listener *listener);
    ~WBaseSession() override;
    // no copyable
    WBaseSession(const WBaseSession& other) = delete;
    WBaseSession& operator=(const WBaseSession& ) = delete;

    // class lifetime
    bool Init(SessionNode *node, WNetWorkHandler *handler);
    bool SetConnectedSocket(base_socket_type socket, const WEndPointInfo &peerInfo);
    // 清理，准备复用
    void Clear();
    void Destroy();
    bool IsConnected();

    virtual bool AcceptConnection(const WEndPointInfo& peer_info) = 0;
    virtual WBaseConnection* CreateConnection() = 0;

protected:
    WBaseSession::Listener *_listener{nullptr};
    SessionNode            *node_{nullptr};
    WNetWorkHandler        *handler_{nullptr};
    WBaseConnection        *connection_{nullptr};
};

extern WBaseSession *CreateNewSession(WBaseSession::Listener *listener);
SessionNode *CreateNewSessionNodeAndInit(WBaseSession::Listener *listener, WNetWorkHandler *handler);

}

