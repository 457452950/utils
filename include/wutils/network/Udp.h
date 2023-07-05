#pragma once
#ifndef UTIL_UDP_H
#define UTIL_UDP_H

#include <cerrno>

#include "IOEvent.h"
#include "Tools.h"

namespace wutils::network::udp {

class Socket {
public:
    Socket() = default;
    ~Socket() {
        if(socket_ != INVALID_SOCKET) {
            ::close(socket_);
            socket_ = INVALID_SOCKET;
        }
    };

    bool Bind(const EndPointInfo &local) {
        this->socket_ = MakeListenedSocket(local);
        if(socket_ == INVALID_SOCKET) {
            return false;
        }

        SetSocketReuseAddr(this->socket_, false);

        return true;
    }
    bool Connect(const EndPointInfo &remote) { return ConnectToHost(this->socket_, remote); }

    int64_t SendTo(const uint8_t *data, uint32_t len, const EndPointInfo &remote) {
        return ::sendto(this->socket_, data, len, 0, remote.AsSockAddr(), remote.GetSockSize());
    }
    int64_t RecvFrom(uint8_t *buffer, uint32_t len, const EndPointInfo &remote) {
        sockaddr_in6 sa{};
        socklen_t    salen{sizeof(sa)};
        return ::recvfrom(this->socket_, buffer, len, 0, (sockaddr *)&sa, &salen);
    }
    int64_t Send(const uint8_t *data, uint32_t len) { return ::send(this->socket_, data, len, 0); }
    int64_t Recv(uint8_t *buffer, uint32_t len) { return recv(this->socket_, buffer, len, 0); }

    socket_t GetNativeSocket() const { return socket_; }
    void     SetNativeSocket(socket_t so) { this->socket_ = so; }

    EndPointInfo GetLocal() {
        EndPointInfo info;

        GetSockName(this->socket_, info);

        return info;
    }

    bool SetPortReuse() { return SetSocketReusePort(this->socket_, true); }
    bool SetNonBlock() { return SetSocketNonBlock(this->socket_, true); }
    bool SetAddrReuse() { return SetSocketReuseAddr(this->socket_, true); }

private:
    socket_t socket_{INVALID_SOCKET};
};

/**
 * UDPPointer
 */
class UDPPointer : public IOEvent {
private:
    explicit UDPPointer(weak_ptr<io_context_t> handle) {
        this->socket_              = make_shared<Socket>();
        this->handler_             = make_shared<io_hdle_t>();
        this->handler_->handle_    = std::move(handle);
        this->handler_->user_data_ = this;
    }

public:
    using message_cb = std::function<void(const EndPointInfo &, const EndPointInfo &, const uint8_t *, uint32_t)>;
    using error_cb   = std::function<void(SystemError)>;

    static shared_ptr<UDPPointer> Create(weak_ptr<io_context_t> handle) {
        auto ptr = shared_ptr<UDPPointer>(new UDPPointer(std::move(handle)));
        return ptr;
    }

    static shared_ptr<UDPPointer> Create(weak_ptr<io_context_t> handle, message_cb acb, error_cb ecb) {
        auto ptr       = shared_ptr<UDPPointer>(new UDPPointer(std::move(handle)));
        ptr->OnMessage = std::move(acb);
        ptr->OnError   = std::move(ecb);
        return ptr;
    }

    static shared_ptr<UDPPointer> Create(weak_ptr<io_context_t> handle, message_cb &&acb, error_cb &&ecb) {
        auto ptr       = shared_ptr<UDPPointer>(new UDPPointer(std::move(handle)));
        ptr->OnMessage = std::forward<UDPPointer::message_cb>(acb);
        ptr->OnError   = std::forward<UDPPointer::error_cb>(ecb);
        return ptr;
    }

    ~UDPPointer() override {
        if(this->handler_) {
            if(this->handler_->IsEnable())
                this->handler_->DisEnable();
        }
    }

    bool Bind(const EndPointInfo &local_endpoint) {
        this->local_endpoint_ = local_endpoint;

        auto ok = socket_->Bind(local_endpoint_);
        if(!ok) {
            return false;
        }

        socket_->SetNonBlock();
        socket_->SetAddrReuse();

        this->handler_->socket_ = socket_->GetNativeSocket();
        this->handler_->SetEvents(HandlerEventType::EV_IN);

        return true;
    }

    message_cb OnMessage;
    error_cb   OnError;

    // unreliable
    bool SendTo(const uint8_t *send_message, uint32_t message_len, const EndPointInfo &remote) {
        assert(this->handler_->IsEnable());
        assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);

        auto len = socket_->SendTo(send_message, message_len, remote);
        if(len == -1) {
            this->onErr(SystemError::GetSysErrCode());
            return false;
        }

        return true;
    }

private:
    void IOIn() final {
        EndPointInfo ei;
        uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};

        auto recv_len = this->socket_->RecvFrom(buf, MAX_UDP_BUFFER_LEN, ei);

        if(recv_len <= 0) { // error
            onErr(SystemError::GetSysErrCode());
        } else {
            if(!OnMessage) {
                return;
            }

            OnMessage(local_endpoint_, ei, buf, recv_len);
        }
    }
    void IOOut() final{};

    void onErr(SystemError err) {
        if(err.Code() != 0) {
            if(this->OnError) {
                this->OnError(err);
            }
        }
    }

private:
    io_hdle_p          handler_{nullptr};
    shared_ptr<Socket> socket_;
    EndPointInfo       local_endpoint_;
};

/***********************************************************
 * UDPChannel
 ************************************************************/
class UDPChannel : public IOEvent {
public:
    explicit UDPChannel(weak_ptr<io_context_t> handle);
    ~UDPChannel() override;

    bool Start(const EndPointInfo &local_ep, const EndPointInfo &remote_ep, bool shared);

    // listener
public:
    class Listener {
    public:
        virtual void OnMessage(const uint8_t *message, uint64_t message_len) = 0;
        virtual void OnError(SystemError)                                    = 0;
    };

    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }

protected:
    weak_ptr<Listener> listener_;

public:
    // unreliable
    bool Send(const uint8_t *send_message, uint32_t message_len);

private:
    void IOIn() final;
    void IOOut() final{};

    void onErr(SystemError);

protected:
    io_hdle_p    handler_;
    EndPointInfo local_endpoint_;
    EndPointInfo remote_endpoint_;
};

/***********************************************************
 * UDPChannel
 ************************************************************/

UDPChannel::UDPChannel(std::weak_ptr<io_context_t> handle) {
    this->handler_             = make_shared<io_hdle_t>();
    this->handler_->user_data_ = this;
    this->handler_->handle_    = std::move(handle);
}

UDPChannel::~UDPChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

bool UDPChannel::Start(const EndPointInfo &local_ep, const EndPointInfo &remote_ep, bool shared) {

    this->local_endpoint_  = local_ep;
    this->remote_endpoint_ = remote_ep;

    auto socket = MakeBindedSocket(local_endpoint_, shared);
    if(socket == -1) {
        return false;
    }


    std::ignore = ConnectToHost(socket, remote_endpoint_);

    this->handler_->socket_ = socket;
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();

    return true;
}

void UDPChannel::IOIn() {
    std::cout << "UDPChannel channel in" << std::endl;

    EndPointInfo ei;
    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};

    // auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);
    auto recv_len = recv(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, 0);

    // if(ei != this->remote_endpoint_) {
    //     std::cout << "not wanted remote " << std::endl;
    //     return;
    // }

    if(recv_len <= 0) { // error
        onErr(SystemError::GetSysErrCode());
    } else {
        if(listener_.expired()) {
            return;
        }

        listener_.lock()->OnMessage(buf, recv_len);
    }
}

void UDPChannel::onErr(SystemError err) {
    if(err.Code() != 0) {
        if(!listener_.expired()) {
            listener_.lock()->OnError(err);
        }
    }
}

bool UDPChannel::Send(const uint8_t *send_message, uint32_t message_len) {
    assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);

    // auto len = ::sendto(this->handler_->socket_,
    //                     (void *)send_message,
    //                     message_len,
    //                     0,
    //                     remote_endpoint_.GetSockAddr(),
    //                     remote_endpoint_.GetSockSize());
    auto len = send(this->handler_->socket_, send_message, message_len, 0);

    if(len == -1) {
        std::cout << "send to err " << std::endl;
        this->onErr(SystemError::GetSysErrCode());
        return false;
    }

    return true;
}


} // namespace wutils::network::udp


#endif // UTIL_UDP_H
