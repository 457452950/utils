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

        handle_->SetEvents(event::EventType::EV_IN);
    }
    ~Acceptor() override {
        handle_.reset();
        acceptor_.Close();
    }

    bool Open(AF_FAMILY family) { return this->acceptor_.Open(family); }

    bool Start(const NetAddress &info) {
        bool ok = false;

        ok = this->acceptor_.SetNonBlock(true);
        if(!ok) {
            return false;
        }

        ok = this->acceptor_.SetAddrReuse(true);
        if(!ok) {
            return false;
        }

        ok = this->acceptor_.Bind(info);
        if(!ok) {
            return false;
        }

        ok = this->acceptor_.Listen();
        if(!ok) {
            return false;
        }

        if(handle_->IsEnable()) {
            handle_->DisEnable();
        }
        handle_->socket_ = acceptor_.Get();
        handle_->Enable();

        return true;
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

    using Accept_cb =
            std::function<void(const NetAddress &local, const NetAddress &remote, unique_ptr<event::IOHandle> handle)>;
    using Error_cb = std::function<void(SystemError)>;

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
            OnAccept(this->GetLocal(), remote, std::move(handle));
        } else {
            cli_socket.Close();
        }
    }

    void dealError() {
        if(OnError) {
            OnError(SystemError::GetSysErrCode());
        }
    }

private:
    unique_ptr<event::IOHandle> handle_;
    tcp::Acceptor               acceptor_;
};

} // namespace wutils::network

#endif // UTIL_ACCEPTOR_H
