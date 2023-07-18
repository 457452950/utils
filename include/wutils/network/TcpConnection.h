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
    uint8_t *buffer;
    uint32_t buffer_len;
};
struct Data {
    const uint8_t *data;
    uint32_t       bytes;
};

using SHUT_DOWN = tcp::SHUT_DOWN;

class Connection : public event::IOEvent {
public:
    Connection(EndPoint local, EndPoint remote, unique_ptr<event::IOHandle> handle) :
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

    const EndPoint &GetLocalInfo() { return local_; }
    const EndPoint &GetRemoteInfo() { return remote_; }

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
    EndPoint                    local_;
    EndPoint                    remote_;
    tcp::Socket                 socket_;
    unique_ptr<event::IOHandle> handle_;

    // buffer
    wutils::ChainBuffer send_buffer_;
};

/**
 * async connection
 */
class AConnection : public event::IOEvent {};

} // namespace wutils::network

#endif // UTIL_TCPCONNECTION_H
