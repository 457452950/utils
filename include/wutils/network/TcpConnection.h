#pragma once
#include <cstdint>
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
    uint64_t buffer_len{0};
};
struct Data {
    const uint8_t *data{nullptr};
    uint64_t       bytes{0};
};

HEAD_ONLY Data CopyData(const Data &data) {
    Data newData;
    newData.data  = new uint8_t[data.bytes];
    newData.bytes = data.bytes;
    std::copy(data.data, data.data + data.bytes, const_cast<uint8_t *>(newData.data));

    return newData;
}

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

    using SHUT_DOWN = tcp::SHUT_DOWN;
    void ShutDown(SHUT_DOWN how = SHUT_DOWN::RDWR) { this->socket_.ShutDown(how); }

    void Send(const uint8_t *data, uint64_t bytes) {
        if(bytes > MAX_CHANNEL_SEND_SIZE) {
            std::cerr << "warn : bytes{" << bytes << "} is logger then max send size " << MAX_CHANNEL_SEND_SIZE << "."
                      << std::endl;
        }

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
        }

        bytes -= len;
        if(bytes) { // has left
            this->send_buffer_.Write(data + len, bytes - len);
            this->handle_->SetEvents(event::EventType::EV_OUT);
            this->handle_->Enable();
        }
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

        this->send_buffer_.Init(0);
    };
    ~AConnection() override {
        handle_->DisEnable();
        handle_.reset();
        socket_.Close();

        this->send_buffer_.Release();
    }

    class Listener {
    public:
        virtual void OnReceived(uint64_t bytes_recved) = 0;
        virtual void OnSent(uint64_t bytes_sent)       = 0;
        virtual void OnError(SystemError error)        = 0;
        virtual void OnDisconnect()                    = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};

    using SHUT_DOWN = tcp::SHUT_DOWN;
    void ShutDown(SHUT_DOWN how = SHUT_DOWN::RDWR) { this->socket_.ShutDown(how); }

    void ASend(const Data &data) {
        int64_t len = 0;

        if(!this->send_buffer_.IsEmpty()) {
            this->send_buffer_.Write(data.data, data.bytes);

        } else { // try to send

            len = this->socket_.SendSome(data.data, data.bytes);

            // send error
            if(len < 0) {
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

            // send complete
            if(len == data.bytes) {
                handleSent(len);
                return;
            }

            this->send_buffer_.Write(data.data + len, data.bytes - len);
        }

        this->handle_->SetEvents(handle_->GetEvents() | event::EventType::EV_OUT);
        this->handle_->Enable();

        handleSent(len);
    }

    enum ARecvFlag : int8_t {
        NoRecv = -1,
        Once   = 0,
        Keep   = 1,
    };
    void AReceive(Buffer buffer, ARecvFlag flag) {
        this->recv_buffer_ = buffer;
        this->arecv_flag_  = flag;

        if(arecv_flag_ != NoRecv) {
            this->handle_->SetEvents(handle_->GetEvents() | event::EventType::EV_IN);
            this->handle_->Enable();
        } else {
            this->handle_->SetEvents(handle_->GetEvents() & (~event::EventType::EV_IN));
        }
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
    void handleRecv(uint64_t bytes_recved) {
        if(listener_) {
            listener_->OnReceived(bytes_recved);
        }
    }
    void handleSent(uint64_t bytes_sent) {
        if(listener_) {
            listener_->OnSent(bytes_sent);
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

        assert(arecv_flag_ != NoRecv);
        if(arecv_flag_ == Once) {
            auto e = this->handle_->GetEvents();
            this->handle_->SetEvents(e & (~event::EventType::EV_IN));
            arecv_flag_ = NoRecv;
        }

        handleRecv(len);
    }
    void IOOut() override {
        int64_t len = 0;

        if(!this->send_buffer_.IsEmpty()) {
            len = this->socket_.SendSome(this->send_buffer_.PeekRead(), this->send_buffer_.GetReadableBytes());

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

            this->send_buffer_.SkipReadBytes(len);
        }

        if(this->send_buffer_.IsEmpty()) {
            auto flag = this->handle_->GetEvents();
            this->handle_->SetEvents(flag & (~event::EventType::EV_OUT));
            return;
        }

        handleSent(len);
    }

private:
    NetAddress local_;
    NetAddress remote_;

    tcp::Socket                 socket_;
    unique_ptr<event::IOHandle> handle_;

    // buffer
    Buffer              recv_buffer_;
    wutils::ChainBuffer send_buffer_;
    // extra
    ARecvFlag           arecv_flag_{NoRecv};
};

} // namespace wutils::network

#endif // UTIL_TCPCONNECTION_H
