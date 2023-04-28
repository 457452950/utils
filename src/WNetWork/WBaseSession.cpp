#include "WNetWork/WBaseSession.hpp"

namespace wlb::network {

SessionNode *CreateNewSessionNodeAndInit(WBaseSession::Listener *listener, WNetWorkHandler *handler) {
    auto *session = CreateNewSession(listener);
    if(session == nullptr) {
        return nullptr;
    }

    SessionNode *node = new(std::nothrow) SessionNode();
    if(node != nullptr && session->Init(node, handler)) {
        node->val = session;
        return node;
    }
    return nullptr;
}

WBaseSession::WBaseSession(WBaseSession::Listener *listener) : _listener(listener) { assert(this->_listener); }

void WBaseSession::Clear() {
    if(this->connection_ != nullptr) {
        this->connection_->Clear();
    }
    this->_listener->OnSessionClosed(this->node_);
}
bool WBaseSession::Init(SessionNode *node, WNetWorkHandler *handler) {
    this->node_       = node;
    this->handler_    = handler;
    this->connection_ = this->CreateConnection();

    if(!this->node_ || !this->handler_ || !this->connection_) {
        return false;
    }

    return true;
}
bool WBaseSession::SetConnectedSocket(socket_t socket, const WEndPointInfo &peerInfo) {
    assert(!this->connection_->isConnected());

    if(!this->AcceptConnection(peerInfo)) {
        return false;
    }
    if(!this->connection_->SetConnectedSocket(socket, peerInfo)) {
        return false;
    }

    this->_listener->OnNewSession(this->node_);

    return true;
}
void WBaseSession::Destroy() {
    if(this->connection_) {
        this->connection_->Clear();
        delete this->connection_;
        this->connection_ = nullptr;
    }
}
WBaseSession::~WBaseSession() { this->Destroy(); }
bool WBaseSession::IsConnected() { return this->connection_->isConnected(); }
} // namespace wlb::network
