#pragma once
#include <cassert>
#include "WConnection.hpp"
#include "../WList.hpp"

namespace wlb::NetWork
{

class WBaseSession;

using SessionList = WList<WBaseSession*>;
using SessionNode = SessionList::WListNode;

class WBaseSession : public WBaseConnection::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void OnNewSession(SessionNode* session) = 0;
        virtual void OnSessionClosed(SessionNode* session) = 0;
    };
    WBaseSession(WBaseSession::Listener* listener) : _listener(listener) {
        assert(this->_listener);
    };
    virtual ~WBaseSession() {};

    // class life time
    virtual bool Init(SessionNode* node, WNetWorkHandler* handler) = 0;
    virtual void Clear() = 0;

    virtual bool SetConnectedSocket(base_socket_type socket,const WEndPointInfo& peerInfo) = 0;
    virtual bool IsConnected() = 0;

protected:
    WBaseSession::Listener* _listener{nullptr};
};

WBaseSession* CreateNewSession(WBaseSession::Listener* listener);
SessionNode* CreateNewSessionNodeAndInit(WBaseSession::Listener* listener, WNetWorkHandler* handler);








}

