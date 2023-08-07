#pragma once
#ifndef UTIL_UDPPOINT_H
#define UTIL_UDPPOINT_H

#include <cstdint>

#include "base/Definition.h"

namespace wutils::network {

struct Package {
    const uint8_t *data{nullptr};
    uint16_t       bytes{0};
};

struct UdpBuffer {
    uint8_t *buffer{nullptr};
    uint16_t len{0};
};


class UdpPoint : public event::IOEvent {
public:
    explicit UdpPoint(weak_ptr<event::IOContext> context) {
        handle_ = make_unique<event::IOHandle>();

        handle_->listener_ = this;
        handle_->context_  = std::move(context);
    };
    ~UdpPoint() override {
        handle_->DisEnable();
        handle_.reset();
        socket_.Close();
    }

    class Listener {
    public:
        virtual void OnReceiveFrom(Package package, NetAddress remote) = 0;
        virtual void OnError(SystemError error)                        = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};

    bool Open(AF_FAMILY family) { return this->socket_.Open(family); }

    bool Bind(const NetAddress &local) {
        if(this->socket_.Bind(local)) {
            local_   = local;
            is_bind_ = true;

            this->handle_->socket_ = this->socket_;
            this->handle_->SetEvents(event::EventType::EV_IN);
            return this->handle_->Enable();
        }
        return false;
    }
    bool Connect(const NetAddress &remote) {
        auto ok = this->socket_.Connect(remote);
        if(ok) {
            this->is_connect_ = true;
            this->remote_     = remote;
        }
        return ok;
    }

    void Send(const uint8_t *data, uint16_t bytes) {
        assert(this->is_connect_);

        int64_t len = 0;

        len = this->socket_.Send(data, bytes);

        if(len != bytes) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        }
    }
    void Send(Package package) { this->Send(package.data, package.bytes); }

    void SendTo(const uint8_t *data, uint16_t bytes, const NetAddress &remote) {
        int64_t len = 0;

        len = this->socket_.SendTo(data, bytes, remote);

        if(len != bytes) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        }
    }
    void SendTo(Package package, const NetAddress &remote) { this->SendTo(package.data, package.bytes, remote); }

    bool              IsBinded() const { return is_bind_; }
    const NetAddress &GetLocalAddress() { return local_; }
    bool              IsConnected() const { return is_connect_; }
    const NetAddress &GetRemoteAddress() { return remote_; }

private:
    void handleError(const SystemError &error) {
        if(listener_) {
            listener_->OnError(error);
        }
    }
    void handleRecv(const uint8_t *data, uint16_t bytes, const NetAddress &remote) {
        if(listener_) {
            listener_->OnReceiveFrom({data, bytes}, remote);
        }
    }

    void IOIn() override {
        static const int buffer_size = MAX_UDP_BUFFER_LEN;

        auto buffer = make_unique<uint8_t[]>(buffer_size);

        NetAddress remote;

        auto len = this->socket_.RecvFrom(buffer.get(), buffer_size, &remote);

        if(len == -1) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        } else if(len == 0) {
            return;
        }

        handleRecv(buffer.get(), len, remote);
    }
    void IOOut() override { abort(); }

private:
    NetAddress                  local_;
    NetAddress                  remote_;
    bool                        is_bind_{false};
    bool                        is_connect_{false};
    udp::Socket                 socket_;
    unique_ptr<event::IOHandle> handle_;
};

class AUdpPoint : public event::IOEvent {
public:
    explicit AUdpPoint(weak_ptr<event::IOContext> context) {
        handle_ = make_unique<event::IOHandle>();

        handle_->listener_ = this;
        handle_->context_  = std::move(context);
    };
    ~AUdpPoint() override {
        handle_->DisEnable();
        handle_.reset();
        socket_.Close();
    }

    class Listener {
    public:
        virtual void OnReceiveFrom(uint64_t len)                      = 0;
        virtual void OnSentTo(uint64_t len, const NetAddress &remote) = 0;
        virtual void OnError(SystemError error)                       = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};

    bool Open(AF_FAMILY family) {
        auto ok = this->socket_.Open(family);
        if(ok) {
            handle_->socket_ = this->socket_;
        }
        return ok;
    }

    bool Bind(const NetAddress &local) {
        auto ok = this->socket_.Bind(local);
        if(ok) {
            local_   = local;
            is_bind_ = true;
        }
        return ok;
    }
    bool Connect(const NetAddress &remote) {
        auto ok = this->socket_.Connect(remote);
        if(ok) {
            this->is_connect_ = true;
            this->remote_     = remote;
        }
        return ok;
    }

    void ASend(const uint8_t *data, uint16_t bytes) { this->ASend({data, bytes}); }
    void ASend(Package package) {
        assert(this->is_connect_);

        int64_t len = 0;

        len = this->socket_.Send(package.data, package.bytes);

        if(len != package.bytes) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        }
        handleSendto(len, this->remote_);
    }

    void ASendTo(const uint8_t *data, uint16_t bytes, const NetAddress &remote) {
        this->ASendTo({data, bytes}, remote);
    }
    void ASendTo(Package package, const NetAddress &remote) {
        int64_t len = 0;

        len = this->socket_.SendTo(package.data, package.bytes, remote);

        if(len != package.bytes) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        }
        handleSendto(len, remote);
    }

    enum ARecvFlag : int8_t {
        NoRecv = -1,
        Once   = 0,
        Keep   = 1,
    };
    void AReceive(UdpBuffer buffer, ARecvFlag flag, NetAddress *address) {
        this->recv_buffer_ = buffer;
        this->recv_remote_ = address;

        this->arecv_flag_ = flag;

        if(arecv_flag_ != NoRecv) {
            this->handle_->SetEvents(handle_->GetEvents() | event::EventType::EV_IN);
            this->handle_->Enable();
        } else {
            this->handle_->SetEvents(handle_->GetEvents() & (~event::EventType::EV_IN));
        }
    }

    bool              IsBinded() const { return is_bind_; }
    const NetAddress &GetLocalAddress() { return local_; }
    bool              IsConnected() const { return is_connect_; }
    const NetAddress &GetRemoteAddress() { return remote_; }

private:
    void handleError(const SystemError &error) {
        if(listener_) {
            listener_->OnError(error);
        }
    }
    void handleRecv(uint64_t len) {
        if(listener_) {
            listener_->OnReceiveFrom(len);
        }
    }
    void handleSendto(uint64_t len, const NetAddress &remote) {
        if(listener_) {
            listener_->OnSentTo(len, remote);
        }
    }

    void IOIn() override {
        auto len = this->socket_.RecvFrom(recv_buffer_.buffer, recv_buffer_.len, this->recv_remote_);

        if(len == -1) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        } else if(len == 0) {
            return;
        }

        assert(arecv_flag_ != NoRecv);
        if(arecv_flag_ == Once) {
            auto e = this->handle_->GetEvents();
            this->handle_->SetEvents(e & (~event::EventType::EV_IN));
            arecv_flag_ = NoRecv;
        }

        handleRecv(len);
    }
    void IOOut() override { abort(); }

private:
    NetAddress                  local_;
    NetAddress                  remote_;
    UdpBuffer                   recv_buffer_;
    NetAddress                 *recv_remote_{nullptr};
    ARecvFlag                   arecv_flag_{NoRecv};
    bool                        is_bind_{false};
    bool                        is_connect_{false};
    udp::Socket                 socket_;
    unique_ptr<event::IOHandle> handle_;
};

} // namespace wutils::network

#endif // UTIL_UDPPOINT_H
