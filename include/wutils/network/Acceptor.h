#pragma once
#ifndef UTIL_ACCEPTOR_H
#define UTIL_ACCEPTOR_H

#include <utility>

#include "IOEvent.h"
#include "Tools.h"
#include "wutils/network/base/Native.h"

namespace wutils::network::tcp {

/**
 *
 */
class acceptor {
public:
    acceptor()  = default;
    ~acceptor() = default;

    bool Bind(const EndPoint &local) {
        this->local_endpoint_ = local;

        this->socket_ = MakeListenedSocket(local_endpoint_);
        if(socket_ == INVALID_SOCKET) {
            return false;
        }

        SetSocketReuseAddr(this->socket_, false);

        return true;
    }
    bool     Listen() { return listen(this->socket_, MAX_LISTEN_BACK_LOG) == 0; }
    socket_t Accept(EndPoint &info) { return Accept4(this->socket_, info, SOCK_NONBLOCK); }
    void     Close() {
        if(this->socket_ != INVALID_SOCKET) {
            ::close(this->socket_);
            this->socket_ = INVALID_SOCKET;
        }
    }

    socket_t GetNativeSocket() const { return socket_; }

    const EndPoint &GetLocal() { return local_endpoint_; }

    bool SetPortReuse() { return SetSocketReusePort(this->socket_, true); }
    bool SetNonBlock() { return SetSocketNonBlock(this->socket_, true); }
    bool SetAddrReuse() { return SetSocketReuseAddr(this->socket_, true); }

private:
    socket_t     socket_{INVALID_SOCKET};
    EndPoint     local_endpoint_;
};

/**
 * Acceptor
 */
class Acceptor : public IOReadEvent {
private:
    explicit Acceptor(weak_ptr<io_context_t> handle) {
        this->acceptor_            = make_shared<acceptor>();
        this->handler_             = make_shared<io_hdle_t>();
        this->handler_->handle_    = std::move(handle);
        this->handler_->user_data_ = this;
    }

public:
    using accept_cb = std::function<void(const EndPoint &, const EndPoint &, io_hdle_p)>;
    using error_cb  = std::function<void(SystemError)>;

    static shared_ptr<Acceptor> Create(weak_ptr<io_context_t> handle) {
        auto ptr = shared_ptr<Acceptor>(new Acceptor(std::move(handle)));
        return ptr;
    }

    static shared_ptr<Acceptor> Create(weak_ptr<io_context_t> handle, accept_cb acb, error_cb ecb) {
        auto ptr      = shared_ptr<Acceptor>(new Acceptor(std::move(handle)));
        ptr->OnAccept = std::move(acb);
        ptr->OnError  = std::move(ecb);
        return ptr;
    }

    static shared_ptr<Acceptor> Create(weak_ptr<io_context_t> handle, accept_cb &&acb, error_cb &&ecb) {
        auto ptr      = shared_ptr<Acceptor>(new Acceptor(std::move(handle)));
        ptr->OnAccept = std::forward<Acceptor::accept_cb>(acb);
        ptr->OnError  = std::forward<Acceptor::error_cb>(ecb);
        return ptr;
    }

    ~Acceptor() override {
        if(this->handler_) {
            if(this->handler_->IsEnable())
                this->handler_->DisEnable();
        }
        this->acceptor_->Close();
    }

    bool Bind(const EndPoint &local) {
        auto ok = acceptor_->Bind(local);
        if(!ok) {
            return false;
        }

        acceptor_->SetNonBlock();
        acceptor_->SetAddrReuse();

        this->handler_->socket_ = acceptor_->GetNativeSocket();
        this->handler_->SetEvents(HandlerEventType::EV_IN);

        return true;
    }

    bool Listen() { return acceptor_->Listen(); }

    bool SetPortReuse() { return acceptor_->SetPortReuse(); }

    const EndPoint &GetLocal() { return acceptor_->GetLocal(); }

    accept_cb OnAccept;
    error_cb  OnError;

private:
    void IOIn() final {
        assert(this->handler_);

        EndPoint     ei;
        socket_t     cli = acceptor_->Accept(ei);

        if(cli <= 0) { // error
            if(OnError) {
                OnError(SystemError::GetSysErrCode());
            }
        } else {
            if(!OnAccept) {
                ::close(cli);
                return;
            }

            auto handler     = make_shared<io_hdle_t>();
            handler->socket_ = cli;
            handler->handle_ = this->handler_->handle_;

            OnAccept(acceptor_->GetLocal(), ei, handler);
        }
    }

private:
    io_hdle_p            handler_{nullptr};
    shared_ptr<acceptor> acceptor_;
};


} // namespace wutils::network::tcp

#endif // UTIL_ACCEPTOR_H
