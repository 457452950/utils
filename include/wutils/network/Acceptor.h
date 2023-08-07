#pragma once
#ifndef UTIL_ACCEPTOR_H
#define UTIL_ACCEPTOR_H

#include <functional>

#include "wutils/Error.h"
#include "io_event/IOEvent.h"
#include "io_event/IOContext.h"
#include "easy/Acceptor.h"

namespace wutils::network {

class Acceptor : public event::IOReadEvent {
public:
    explicit Acceptor(weak_ptr<event::IOContext> context) : handle_(make_unique<event::IOHandle>()) {
        handle_->listener_ = this;
        handle_->context_  = std::move(context);
    }
    ~Acceptor() override {
        handle_.reset();
        acceptor_.Close();
    }

    bool Open(AF_FAMILY family) { return this->acceptor_.Open(family); }

    Error Start(const NetAddress &info) {
        bool ok = false;

        ok = this->acceptor_.SetNonBlock(true);
        if(!ok) {
            return GetGenericError();
        }

        ok = this->acceptor_.SetAddrReuse(true);
        if(!ok) {
            return GetGenericError();
        }

        ok = this->acceptor_.Bind(info);
        if(!ok) {
            return GetGenericError();
        }

        ok = this->acceptor_.Listen();
        if(!ok) {
            return GetGenericError();
        }

        if(handle_->IsEnable()) {
            handle_->DisEnable();
        }
        handle_->socket_ = acceptor_.Get();

        return this->handle_->EnableIn(true);
    }

    void Stop() {
        if(handle_->IsEnable()) {
            handle_->DisEnable();
        }
        if(acceptor_) {
            acceptor_.Close();
        }
    }

    const NetAddress &GetLocal() { return acceptor_.GetLocal(); }

    bool SetPortReuse(bool is_set) { return this->acceptor_.SetPortReuse(is_set); }

    using Accept_cb = std::function<void(const NetAddress &remote, unique_ptr<event::IOHandle> handle)>;
    using Error_cb  = std::function<void(Error)>;

    Accept_cb OnAccept;
    Error_cb  OnError;

private:
    void IOIn() override {
        NetAddress remote;
        ISocket    cli_socket = acceptor_.Accept(remote, true);

        if(!cli_socket) {
            dealError();
            return;
        }

        auto handle      = make_unique<event::IOHandle>();
        handle->socket_  = cli_socket;
        handle->context_ = this->handle_->context_;

        assert(handle->socket_);

        if(OnAccept) {
            OnAccept(remote, std::move(handle));
        } else {
            abort();
        }
    }

    void dealError() {
        if(OnError) {
            OnError(GetGenericError());
        }
    }

private:
    unique_ptr<event::IOHandle> handle_;
    tcp::Acceptor               acceptor_;
};

class AAcceptor : public event::IOReadEvent {
public:
    explicit AAcceptor(weak_ptr<event::IOContext> context) : handle_(make_unique<event::IOHandle>()) {
        handle_->listener_ = this;
        handle_->context_  = std::move(context);
    }
    ~AAcceptor() override {
        handle_.reset();
        acceptor_.Close();
    }

    bool Open(AF_FAMILY family) { return this->acceptor_.Open(family); }

    enum AAcceptFlag : int8_t {
        NoAccept = -1,
        Once     = 0,
        Keep     = 1,
    };
    Error Start(const NetAddress &local, NetAddress *remote, AAcceptFlag flag) {
        bool ok = false;

        ok = this->acceptor_.SetNonBlock(true);
        if(!ok) {
            return GetGenericError();
        }

        ok = this->acceptor_.SetAddrReuse(true);
        if(!ok) {
            return GetGenericError();
        }

        ok = this->acceptor_.Bind(local);
        if(!ok) {
            return GetGenericError();
        }

        ok = this->acceptor_.Listen();
        if(!ok) {
            return GetGenericError();
        }

        if(handle_->IsEnable()) {
            handle_->DisEnable();
        }

        handle_->socket_ = acceptor_.Get();

        this->remote_      = remote;
        this->accept_flag_ = flag;


        if(accept_flag_ != NoAccept) {
            return handle_->EnableIn(true);
        } else {
            handle_->DisEnable();
        }

        return eNetWorkError::OK;
    }

    const NetAddress &GetLocal() { return acceptor_.GetLocal(); }

    bool SetPortReuse(bool is_set) { return this->acceptor_.SetPortReuse(is_set); }

    using Accept_cb = std::function<void(unique_ptr<event::IOHandle> handle)>;
    using Error_cb  = std::function<void(Error)>;

    Accept_cb OnAccept;
    Error_cb  OnError;

private:
    void IOIn() override {
        ISocket cli_socket = acceptor_.Accept(*remote_, true);

        if(!cli_socket) {
            dealError();
            return;
        }

        auto handle      = make_unique<event::IOHandle>();
        handle->socket_  = cli_socket;
        handle->context_ = this->handle_->context_;

        assert(handle->socket_);

        if(OnAccept) {
            OnAccept(std::move(handle));
        } else {
            abort();
        }
    }

    void dealError() {
        if(OnError) {
            OnError(GetGenericError());
        }
    }

private:
    unique_ptr<event::IOHandle> handle_;
    tcp::Acceptor               acceptor_;
    AAcceptFlag                 accept_flag_{NoAccept};
    NetAddress                 *remote_{nullptr};
};

} // namespace wutils::network

#endif // UTIL_ACCEPTOR_H
