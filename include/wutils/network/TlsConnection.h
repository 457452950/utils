#pragma once
#ifndef UTIL_TLSCONNECTION_H
#define UTIL_TLSCONNECTION_H

#include "TcpConnection.h"
#include "ssl/ssl.h"

namespace wutils::network {

// TODO:
class TlsConnection : public event::IOEvent {
public:
    TlsConnection(NetAddress                  local,
                  NetAddress                  remote,
                  unique_ptr<event::IOHandle> handle,
                  shared_ptr<ssl::SslContext> ssl_context) :
        local_(std::move(local)),
        remote_(std::move(remote)), handle_(std::move(handle)), n_socket_(handle_->socket_),
        ssl_socket_(ssl_context->NewSocket(handle_->socket_)) {

        handle_->listener_ = this;
        assert(n_socket_);
    }

    ~TlsConnection() override {
        handle_->DisEnable();
        handle_.reset();
        ssl_socket_.Close();
        n_socket_.Close();
    }

    class Listener {
    public:
        virtual void OnDisconnect()       = 0;
        virtual void OnReceive(Data data) = 0;
        virtual void OnError(Error error) = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};

    Error Init() {
        auto err = handle_->EnableIn(true);
        if(err) {
            return err;
        }
        err = ssl_socket_.Open();
        if(err) {
            return err;
        }
        err = ssl_socket_.SslDoHandShake();
        if(err) {
            return err;
        }
        return {};
    }

    void ShutDown() { this->ssl_socket_.SslShutDown(); }

    void Send(const uint8_t *data, uint64_t bytes) {
        if(bytes > MAX_CHANNEL_SEND_SIZE) {
            handleError(eNetWorkError::SEND_DATA_TOO_LONG);
            return;
        }

        Error err{};
        err = this->ssl_socket_.SslWrite(data, bytes);

        if(err) {
            handleError(err);
            return;
        }
    }
    void Send(Data data) { this->Send(data.data, data.bytes); }

    const NetAddress &GetLocalAddress() { return local_; }
    const NetAddress &GetRemoteAddress() { return remote_; }

private:
    void handleError(Error error) {
        switch(error.value()) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_ACCEPT:
            error = this->handle_->EnableIn(true);
            break;
        case SSL_ERROR_WANT_WRITE:
            error = this->handle_->EnableOut(true);
            break;
        case SSL_ERROR_SYSCALL:
            error = GetGenericError();
            break;
        }

        if(error) {
            if(listener_) {
                listener_->OnError(error);
            }
        }
    }
    void handleDisconnection() {
        if(listener_) {
            listener_->OnDisconnect();
        }
    }
    void handleRecv(const uint8_t *data, uint32_t bytes) {
        if(listener_) {
            listener_->OnReceive({data, bytes});
        }
    }

    void IOIn() override {
        static const int buffer_size = 4 * 1024;

        auto buffer     = make_unique<uint8_t[]>(buffer_size);
        auto [len, err] = this->ssl_socket_.SslRead(buffer.get(), buffer_size);

        if(err) {
            handleError(err);
            return;
        }

        if(len > 0)
            handleRecv(buffer.get(), len);
    }
    void IOOut() override { this->IOIn(); }

private:
    NetAddress                  local_;
    NetAddress                  remote_;
    unique_ptr<event::IOHandle> handle_;
    ISocket                     n_socket_;
    ssl::Socket                 ssl_socket_;
};

/**
 * async connection
 */
// class AConnection : public event::IOEvent {
// public:
//     AConnection(NetAddress local, NetAddress remote, unique_ptr<event::IOHandle> handle) :
//         local_(std::move(local)), remote_(std::move(remote)), handle_(std::move(handle)) {
//         handle_->listener_ = this;
//
//         socket_ = handle_->socket_;
//         assert(socket_);
//
//         this->send_buffer_.Init(0);
//     };
//     ~AConnection() override {
//         handle_->DisEnable();
//         handle_.reset();
//         socket_.Close();
//
//         this->send_buffer_.Release();
//     }
//
//     class Listener {
//     public:
//         virtual void OnReceived(uint64_t bytes_recved) = 0;
//         virtual void OnSent(uint64_t bytes_sent)       = 0;
//         virtual void OnError(Error error)              = 0;
//         virtual void OnDisconnect()                    = 0;
//
//         virtual ~Listener() = default;
//     } *listener_{nullptr};
//
//     using SHUT_DOWN = tcp::SHUT_DOWN;
//     void ShutDown(SHUT_DOWN how = SHUT_DOWN::RDWR) { this->socket_.ShutDown(how); }
//
//     void ASend(const Data &data) {
//         int64_t len = 0;
//
//         if(!this->send_buffer_.IsEmpty()) {
//             this->send_buffer_.Write(data.data, data.bytes);
//
//         } else { // try to send
//             len = this->socket_.SendSome(data.data, data.bytes);
//
//             // send error
//             if(len < 0) {
//                 auto err = GetGenericError();
//                 if(err.value() != EAGAIN && err.value() != EWOULDBLOCK && err.value() != EINTR) {
//                     handleError(err);
//                     return;
//                 }
//                 len = 0;
//             }
//             // connection close
//             else if(len == 0) {
//                 handleDisconnection();
//                 return;
//             }
//
//             // send complete
//             if(len == data.bytes) {
//                 handleSent(len);
//                 return;
//             }
//
//             this->send_buffer_.Write(data.data + len, data.bytes - len);
//         }
//
//         auto err = this->handle_->EnableOut(true);
//         if(err) {
//             handleError(err);
//             return;
//         }
//
//         handleSent(len);
//     }
//
//     enum ARecvFlag : int8_t {
//         NoRecv = -1,
//         Once   = 0,
//         Keep   = 1,
//     };
//     void AReceive(Buffer buffer, ARecvFlag flag) {
//         this->recv_buffer_ = buffer;
//         this->arecv_flag_  = flag;
//
//         Error err;
//         if(arecv_flag_ != NoRecv) {
//             err = this->handle_->EnableIn(true);
//         } else {
//             err = this->handle_->EnableIn(false);
//         }
//         if(err) {
//             handleError(err);
//             return;
//         }
//     }
//
//     const NetAddress &GetLocalAddress() { return local_; }
//     const NetAddress &GetRemoteAddress() { return remote_; }
//
// private:
//     void handleError(const Error &error) {
//         if(listener_) {
//             listener_->OnError(error);
//         }
//     }
//     void handleDisconnection() {
//         if(listener_) {
//             listener_->OnDisconnect();
//         }
//     }
//     void handleRecv(uint64_t bytes_recved) {
//         if(listener_) {
//             listener_->OnReceived(bytes_recved);
//         }
//     }
//     void handleSent(uint64_t bytes_sent) {
//         if(listener_) {
//             listener_->OnSent(bytes_sent);
//         }
//     }
//
//     void IOIn() override {
//         auto len = this->socket_.Recv(recv_buffer_.buffer, recv_buffer_.buffer_len);
//
//         if(len == -1) {
//             auto err = GetGenericError();
//             if(err.value() != EAGAIN && err.value() != EWOULDBLOCK && err.value() != EINTR) {
//                 handleError(err);
//             }
//             return;
//         } else if(len == 0) {
//             handleDisconnection();
//             return;
//         }
//
//         assert(arecv_flag_ != NoRecv);
//         if(arecv_flag_ == Once) {
//             arecv_flag_ = NoRecv;
//             auto err    = this->handle_->EnableIn(false);
//             if(err) {
//                 handleError(err);
//                 return;
//             }
//         }
//
//         handleRecv(len);
//     }
//     void IOOut() override {
//         int64_t len = 0;
//
//         if(!this->send_buffer_.IsEmpty()) {
//             len = this->socket_.SendSome(this->send_buffer_.PeekRead(), this->send_buffer_.GetReadableBytes());
//
//             // send error
//             if(len == -1) {
//                 auto err = GetGenericError();
//                 if(err.value() != EAGAIN && err.value() != EWOULDBLOCK && err.value() != EINTR) {
//                     handleError(err);
//                     return;
//                 }
//                 len = 0;
//             }
//             // connection close
//             else if(len == 0) {
//                 handleDisconnection();
//                 return;
//             }
//
//             this->send_buffer_.SkipReadBytes(len);
//         }
//
//         if(this->send_buffer_.IsEmpty()) {
//             auto err = this->handle_->EnableOut(false);
//             if(err) {
//                 handleError(err);
//                 return;
//             }
//         }
//
//         handleSent(len);
//     }
//
// private:
//     NetAddress local_;
//     NetAddress remote_;
//
//     tcp::Socket                 socket_;
//     unique_ptr<event::IOHandle> handle_;
//
//     // buffer
//     Buffer              recv_buffer_;
//     wutils::ChainBuffer send_buffer_;
//     // extra
//     ARecvFlag           arecv_flag_{NoRecv};
// };

} // namespace wutils::network

#endif // UTIL_TCPCONNECTION_H
