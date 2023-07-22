#pragma once
#ifndef UTIL_TCPCONNECTION_H
#define UTIL_TCPCONNECTION_H

#include <utility>

#include "wutils/Error.h"
#include "wutils/Buffer.h"
#include "easy/Tcp.h"
#include "io_event/IOEvent.h"
#include "io_event/IOContext.h"

namespace wutils::network {

struct Buffer {
    uint8_t *buffer{nullptr};
    uint32_t buffer_len{0};
};
struct Data {
    const uint8_t *data{nullptr};
    uint32_t       bytes{0};
};

using SHUT_DOWN = tcp::SHUT_DOWN;

class Connection : public event::IOEvent {
public:
    Connection(NetAddress local, NetAddress remote, unique_ptr<event::IOHandle> handle) :
        local_(std::move(local)), remote_(std::move(remote)), handle_(std::move(handle)) {
        handle_->listener_ = this;

        socket_ = handle_->socket_;
        assert(socket_);

        handle_->SetEvents(event::EventType::EV_IN);
        handle_->Enable();

        this->send_buffer_.Init(0);
    };
    ~Connection() override {
        handle_->DisEnable();
        handle_.reset();
        socket_.Close();
    }

    class Listener {
    public:
        virtual void OnDisconnect()             = 0;
        virtual void OnReceive(Data data)       = 0;
        virtual void OnError(SystemError error) = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};

    void ShutDown(SHUT_DOWN how = SHUT_DOWN::RDWR) { this->socket_.ShutDown(how); }

    void Send(const uint8_t *data, uint32_t bytes) {
        int64_t len = 0;
        if(send_buffer_.IsEmpty()) {
            len = this->socket_.SendSome(data, bytes);
        }

        if(len == -1) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
                return;
            }
            len = 0;
        } else if(len == 0) {
            handleDisconnection();
            return;
        } else if(len == bytes) {
            // sent all
            return;
        }

        this->send_buffer_.Write(data + len, bytes - len);
        this->handle_->SetEvents(event::EventType::EV_OUT);
        this->handle_->Enable();
    }
    void Send(Data data) { this->Send(data.data, data.bytes); }

    const NetAddress &GetLocalAddress() { return local_; }
    const NetAddress &GetRemoteAddress() { return remote_; }

private:
    void handleError(const SystemError &error) {
        if(listener_) {
            listener_->OnError(error);
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

        auto buffer = make_unique<uint8_t[]>(buffer_size);
        auto len    = this->socket_.Recv(buffer.get(), buffer_size);

        if(len == -1) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        } else if(len == 0) {
            handleDisconnection();
            return;
        }

        handleRecv(buffer.get(), len);
    }
    void IOOut() override {
        if(!this->send_buffer_.IsEmpty()) {
            auto len = this->socket_.SendSome(send_buffer_.PeekRead(), send_buffer_.GetReadableBytes());
            this->send_buffer_.SkipReadBytes(len);
        }

        if(this->send_buffer_.IsEmpty()) {
            auto flag = this->handle_->GetEvents();
            this->handle_->SetEvents(flag & (~event::EventType::EV_OUT));
            return;
        }
    }

private:
    NetAddress                  local_;
    NetAddress                  remote_;
    tcp::Socket                 socket_;
    unique_ptr<event::IOHandle> handle_;

    // buffer
    wutils::ChainBuffer send_buffer_;
};

/**
 * async connection
 */
class AConnection : public event::IOEvent {
public:
    AConnection(NetAddress local, NetAddress remote, unique_ptr<event::IOHandle> handle) :
        local_(std::move(local)), remote_(std::move(remote)), handle_(std::move(handle)) {
        handle_->listener_ = this;

        socket_ = handle_->socket_;
        assert(socket_);
    };
    ~AConnection() override {
        handle_->DisEnable();
        handle_.reset();
        socket_.Close();
    }

    class Listener {
    public:
        virtual void OnDisconnect()             = 0;
        virtual void OnReceived(Buffer buffer)  = 0;
        virtual void OnSent(Buffer buffer)      = 0;
        virtual void OnError(SystemError error) = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};

    void ShutDown(SHUT_DOWN how = SHUT_DOWN::RDWR) { this->socket_.ShutDown(how); }

    void ASend(const Buffer &buffer) {
        int64_t len = 0;

        if(!this->send_buffers_.empty()) {
            this->send_buffers_.push_back(buffer);
        } else { // try to send

            send_index_ = this->socket_.SendSome(buffer.buffer, buffer.buffer_len);

            // send complete
            if(send_index_ == buffer.buffer_len) {
                handleSent(buffer);
                return;
            }
            // send error
            else if(send_index_ == -1) {
                auto err = SystemError::GetSysErrCode();
                if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                    handleError(err);
                    return;
                }
                send_index_ = 0;
            }
            // connection close
            else if(send_index_ == 0) {
                handleDisconnection();
                return;
            }

            //
            this->send_buffers_.push_back(buffer);
        }

        this->handle_->SetEvents(handle_->GetEvents() | event::EventType::EV_OUT);
        this->handle_->Enable();
    }

    enum ARecvFlag : int8_t { NoRecv = -1, Once = 0, Keep = 1 };

    void AReceive(Buffer buffer, ARecvFlag flag = Once) {
        this->recv_buffer_ = buffer;
        this->arecv_flag_  = flag;

        this->handle_->SetEvents(handle_->GetEvents() | event::EventType::EV_IN);
        this->handle_->Enable();
    }

    const NetAddress &GetLocalAddress() { return local_; }
    const NetAddress &GetRemoteAddress() { return remote_; }

private:
    void handleError(const SystemError &error) {
        if(listener_) {
            listener_->OnError(error);
        }
    }
    void handleDisconnection() {
        if(listener_) {
            listener_->OnDisconnect();
        }
    }
    void handleRecv(const Buffer &buffer) {
        if(listener_) {
            listener_->OnReceived(buffer);
        }
    }
    void handleSent(const Buffer &buffer) {
        if(listener_) {
            listener_->OnSent(buffer);
        }
    }

    void IOIn() override {
        auto len = this->socket_.Recv(recv_buffer_.buffer, recv_buffer_.buffer_len);

        if(len == -1) {
            auto err = SystemError::GetSysErrCode();
            if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                handleError(err);
            }
            return;
        } else if(len == 0) {
            handleDisconnection();
            return;
        }

        if(arecv_flag_ == Once)
            arecv_flag_ = NoRecv;

        handleRecv({.buffer = recv_buffer_.buffer, .buffer_len = (uint32_t)len});

        if(arecv_flag_ == NoRecv) {
            auto e = this->handle_->GetEvents();
            this->handle_->SetEvents(e & (~event::EventType::EV_IN));
        }
    }
    void IOOut() override {
        while(!this->send_buffers_.empty()) {
            auto it = this->send_buffers_.begin();

            auto len = this->socket_.SendSome(it->buffer + send_index_, it->buffer_len - send_index_);

            // send error
            if(len == -1) {
                auto err = SystemError::GetSysErrCode();
                if(err != EAGAIN && err != EWOULDBLOCK && err != EINTR) {
                    handleError(err);
                    return;
                }
                len = 0;
            }
            // connection close
            else if(len == 0) {
                handleDisconnection();
                return;
            }

            send_index_ += len;
            // send complete
            if(send_index_ == it->buffer_len) {
                this->send_buffers_.erase(it);
                send_index_ = 0;
            } else {
                break;
            }
        }

        if(this->send_buffers_.empty()) {
            auto flag = this->handle_->GetEvents();
            this->handle_->SetEvents(flag & (~event::EventType::EV_OUT));
            return;
        }
    }

private:
    NetAddress local_;
    NetAddress remote_;

    tcp::Socket                 socket_;
    unique_ptr<event::IOHandle> handle_;

    // buffer
    Buffer            recv_buffer_;
    std::list<Buffer> send_buffers_;
    // extra
    ARecvFlag         arecv_flag_;
    uint32_t          send_index_{0};
};

} // namespace wutils::network

#endif // UTIL_TCPCONNECTION_H
