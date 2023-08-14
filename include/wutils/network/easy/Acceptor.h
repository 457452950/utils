#pragma once
#ifndef UTIL_EASY_ACCEPTOR_H
#define UTIL_EASY_ACCEPTOR_H

#include "Tcp.h"

namespace wutils::network::tcp {

/**
 *
 */
class Acceptor {
public:
    Acceptor()  = default;
    ~Acceptor() = default;

    operator bool() const { return accept_socket_.operator bool(); }

    // Common
    bool Open(AF_FAMILY family) { return this->accept_socket_.Open(family); }

    bool Bind(const NetAddress &local) {
        this->local_endpoint_ = local;
        return this->accept_socket_.Bind(this->local_endpoint_);
    }

    bool Listen() { return this->accept_socket_.Listen(); }

    ISocket Accept(NetAddress *info, bool set_nonblock = false) {
        return this->accept_socket_.Accept(info, set_nonblock);
    }

    void Close() { this->accept_socket_.Close(); }

    socket_t Get() const { return accept_socket_.Get(); }

    const NetAddress &GetLocal() { return local_endpoint_; }

    bool IsNonBlock() { return this->accept_socket_.IsNonBlock(); }
    bool SetNonBlock(bool is_set) { return this->accept_socket_.SetNonBlock(is_set); }
    bool SetPortReuse(bool is_set) { return this->accept_socket_.SetPortReuse(is_set); }
    bool SetAddrReuse(bool is_set) { return this->accept_socket_.SetAddrReuse(is_set); }

private:
    Socket     accept_socket_;
    NetAddress local_endpoint_;
};

} // namespace wutils::network::tcp

#endif // UTIL_EASY_ACCEPTOR_H
