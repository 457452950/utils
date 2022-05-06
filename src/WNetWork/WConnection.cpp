#include "WNetWork/WConnection.hpp"
#include <iostream>
#include <cassert>

namespace wlb::NetWork {

/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////


WNetAccepter::WNetAccepter(Listener *listener) : listener_(listener) {
}

WNetAccepter::~WNetAccepter() {
    this->Close();
}

bool WNetAccepter::Init(WNetWorkHandler *handler, const std::string &IpAddress, uint16_t port) {
    this->Init(handler, {IpAddress, port, true});
}

bool WNetAccepter::Listen() {
    if (::listen(this->socket_, 1024) == -1) {
        this->errno_ = errno;
        return false;
    }
    return true;
}

base_socket_type WNetAccepter::Accept(WEndPointInfo *info) {
    return wlb::NetWork::Accept(this->socket_, info, true);
}

base_socket_type WNetAccepter::GetListenSocket() noexcept {
    return this->socket_;
}

void WNetAccepter::Close() {
    this->handler_->RemoveSocket(this->socket_);

    if (this->socket_ != -1) {
        ::close(this->socket_);
        this->socket_ = -1;
    }

    this->handler_ = nullptr;

    this->listener_ = nullptr;

    delete this->handler_data_;
    this->handler_data_ = nullptr;
}

void WNetAccepter::OnRead() {
    WEndPointInfo    info;
    base_socket_type cli_sock = this->Accept(&info);

    if (cli_sock == -1) {
        this->errno_ = errno;
        return;
    }

    if (this->listener_ != nullptr) {
        if (!this->listener_->OnNewConnection(cli_sock, info)) {
            ::close(cli_sock);
            std::cout << "close client" << info.ip_address << " " << info.port << std::endl;
        }
    }

}

void WNetAccepter::OnError(int16_t error_code) {
    this->errno_ = error_code;
}

int16_t WNetAccepter::GetErrorNo() noexcept {
    int16_t e = this->errno_;
    this->errno_ = -1;
    return e;
}
bool WNetAccepter::Init(WNetWorkHandler *handler, const WEndPointInfo &end_point_info) {
    this->socket_     = MakeTcpV4Socket();       // tcp v4
    if (this->socket_ == -1) {
        return false;
    }

    if (!SetSocketReuseAddr(this->socket_) ||
            !SetSocketReusePort(this->socket_) ||
            !SetSocketKeepAlive(this->socket_)) {
        this->errno_ = errno;
        return false;
    }

    if (!SetSocketNoBlock(this->socket_)) {
        this->errno_ = errno;
    }
    // ///////////////////////////////////////
    // Init members
    this->local_info_ = end_point_info;

    this->handler_ = handler;
    assert(this->handler_);

    // ///////////////////////////////////////
    // Bind
    if (!wlb::NetWork::Bind(this->socket_, this->local_info_)) {
        this->errno_ = errno;
        return false;
    }
    /////////////////////////////////
    // Listen
    if (!this->Listen()) {
        return false;
    }

    this->handler_data_ = new(std::nothrow) WHandlerData(this->socket_, this);
    assert(this->handler_data_);

    // //////////////////////////////
    // add socket in handler
    uint32_t op = 0;
    op |= WNetWorkHandler::OP_IN;
    op |= WNetWorkHandler::OP_ERR;

    if (!this->handler_->AddSocket(this->handler_data_, op)) {
        this->errno_ = this->handler_->GetErrorNo();
        return false;
    }

    return true;
}
const WEndPointInfo &WNetAccepter::GetLocalInfo() const {
    return local_info_;
}






/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Seession
////////////////////////////////////////////////////////

WFloatBufferConnection::WFloatBufferConnection(WBaseConnection::Listener *listener) :
        listener_(listener) {
}
WFloatBufferConnection::~WFloatBufferConnection() {
    this->Destroy();
}

bool WFloatBufferConnection::Init(WNetWorkHandler *handler, uint32_t maxBufferSize, uint32_t headLen) {
    this->max_buffer_size_ = maxBufferSize;
    this->head_len_        = headLen;

    if (!recv_buffer_.Init(maxBufferSize)) {
        return false;
    }
    if (!send_buffer_.Init(maxBufferSize)) {
        return false;
    }

    this->handler_ = handler;
    assert(this->handler_);

    this->op_ |= WNetWorkHandler::OP_IN;
    this->op_ |= WNetWorkHandler::OP_ERR;
    this->op_ |= WNetWorkHandler::OP_SHUT;
    this->op_ |= WNetWorkHandler::OP_CLOS;

    this->handler_data_ = new(std::nothrow) WHandlerData(this->socket_, this);
    assert(this->handler_data_);

    return true;
}

bool WFloatBufferConnection::SetConnectedSocket(base_socket_type socket, const WEndPointInfo &peerInfo) {
    this->socket_ = socket;

    if (!SetTcpSocketNoDelay(this->socket_) ||
            !SetSocketKeepAlive(this->socket_)) {
        this->error_code_ = errno;
        return false;
    }
    if (!SetSocketNoBlock(this->socket_)) {
        this->error_code_ = errno;
    }
    if (!handler_->AddSocket(this->handler_data_, this->op_)) {
        this->error_code_ = this->handler_->GetErrorNo();
        return false;
    }

    this->peer_info_ = peerInfo;

    this->is_connected_ = true;

    return true;
}

void WFloatBufferConnection::CloseConnection() {
    if (::shutdown(this->socket_, SHUT_RDWR) == -1) {
        this->error_code_ = errno;
        this->listener_->OnConnectionError();
    }
}

void WFloatBufferConnection::Clear() {
    if (!this->is_connected_) {
        return;
    }

    this->is_connected_ = false;

    this->handler_->RemoveSocket(this->socket_);

    if (this->socket_ != -1) {
        ::close(this->socket_);
        this->socket_ = -1;
    }

    recv_buffer_.Clear();
    send_buffer_.Clear();
}

void WFloatBufferConnection::Destroy() {
    this->Clear();

    recv_buffer_.Destroy();
    send_buffer_.Destroy();

    this->listener_ = nullptr;

    delete this->handler_data_;
    this->handler_data_ = nullptr;
}

bool WFloatBufferConnection::Send(const std::string &message) {
    uint32_t msgSize    = message.size();
    uint32_t insert_len = 0;

    std::string send_message;
    if (this->head_len_ == 2) {
        wlbHead<2> head{msgSize};
        send_message.append(reinterpret_cast<char *>(head.data_uchar));
    } else if (this->head_len_ == 4) {
        wlbHead<4> head{msgSize};
        send_message.append(reinterpret_cast<char *>(head.data_uchar));
    }

    send_message.append(message);
    insert_len = send_buffer_.InsertMessage(send_message);
    if (insert_len != send_message.length()) {
        std::cout << "insert message false" << std::endl;
        return false;
    }

    std::cout << "send message start set epoll out" << std::endl;
    if (!(this->op_ & WNetWorkHandler::OP_OUT)) {
        std::cout << "send message set epoll out" << std::endl;
        // 添加进 send events
        this->op_ |= WNetWorkHandler::OP_OUT;
        if (!handler_->ModifySocket(this->handler_data_, this->op_)) {
            std::cout << "send message set epoll out failed" << std::endl;
            this->error_code_ = this->handler_->GetErrorNo();
            std::cout << "send message set epoll out failed" << strerror(this->error_code_) << std::endl;
            return false;
        }
    }
    std::cout << "WFloatBufferConnection::Send end" << std::endl;
    return true;
}

bool WFloatBufferConnection::Receive() {
    std::cout << "WFloatBufferConnection::Receive" << std::endl;
    int64_t recv_len = ::recv(this->socket_,
                              this->recv_buffer_.GetRestBuffer(),
                              this->recv_buffer_.GetTopRestBufferSize(),
                              0);
    if (recv_len <= -1) {
        this->HandleError(errno);
        std::cout << this->error_code_ << std::endl;
        this->listener_->OnConnectionError();
        return false;
    }
    if (recv_len == 0 && recv_buffer_.GetTopRestBufferSize() != 0) {
        this->listener_->OnConnectionClosed();
        std::cout << "recv 0" << std::endl;
        return false;
    }

    this->recv_buffer_.UpdateWriteOffset(recv_len);

    return true;
}

void WFloatBufferConnection::HandleError(int16_t error_code) {
    this->error_code_ = error_code;
}

void WFloatBufferConnection::OnError(int16_t error_no) {
    HandleError(error_no);

    if (this->listener_ != nullptr) {
        this->listener_->OnConnectionError();
    }
}

void WFloatBufferConnection::OnClosed() {
    if (this->listener_ != nullptr) {
        this->listener_->OnConnectionClosed();
    }
}

void WFloatBufferConnection::OnRead() {
    if (!this->Receive()) {
        return;
    }
    std::cout << "WFloatBufferConnection::OnRead" << std::endl;

    std::string head;
    std::string receive_message;
    // on message
    while (true) {
        receive_message.clear();
        uint32_t len = this->recv_buffer_.GetFrontMessage(&head, this->head_len_);

        if (len != this->head_len_) {
            std::cout << "WFloatBufferConnection::OnRead no enough message len:" << len << std::endl;
            break;
        }

        len = GetLengthFromWlbHead(head.c_str(), this->head_len_);
        std::cout << "WFloatBufferConnection::OnRead head body length:" << len << std::endl;

        if (len == 0) {
            this->CloseConnection();
            std::cout << "Invalid message head len = 0" << std::endl;
            return;
        }

        if (len > this->recv_buffer_.GetFrontMessageLength()) {
            std::cout << "WFloatBufferConnection::OnRead no enough message len" << len << std::endl;
            return;
        }

        this->recv_buffer_.UpdateReadOffset(this->head_len_);

        len = this->recv_buffer_.GetFrontMessage(&receive_message, len);
        if (len == 0) {
            // no enough message
            std::cout << "WFloatBufferConnection::OnRead error no enough message len" << len << std::endl;
            break;
        }

        (len > 0) ? this->recv_buffer_.UpdateReadOffset(len)
                  : this->recv_buffer_.UpdateReadOffset(0);

        this->listener_->OnConnectionMessage(receive_message);
    }
}

void WFloatBufferConnection::OnWrite() {
    std::cout << "WFloatBufferConnection::OnWrite()" << std::endl;
    std::string send_message;
    uint32_t    msg_len  = send_buffer_.GetAllMessage(&send_message);
    ssize_t     send_len = ::send(this->socket_, send_message.c_str(), msg_len, 0);

    std::cout << "send len :" << send_len << std::endl;

    this->send_buffer_.UpdateReadOffset(send_len);

    if (send_len < 0) {
        this->HandleError(errno);
        this->listener_->OnConnectionError();
        return;
    }

    if (send_buffer_.Empty()) {
        this->op_ -= WNetWorkHandler::OP_OUT;
        this->handler_->ModifySocket(this->handler_data_, this->op_);
    }
}

void WFloatBufferConnection::OnShutdown() {
    if (this->listener_ != nullptr) {
        this->listener_->OnConnectionShutdown();
    }

}
int WFloatBufferConnection::GetErrorCode() {
    auto e = this->error_code_;
    this->error_code_ = -1;
    return e;
}
const WEndPointInfo &WFloatBufferConnection::GetPeerInfo() {
    return this->peer_info_;
}

// wlb style methods
uint64_t GetLengthFromWlbHead(const char *str_data, uint8_t head_length) {
    if (head_length == 0) {
        assert(head_length);
    } else if (head_length == 2) {
        wlbHead<2> data{};
        memccpy(data.data_uchar, str_data, head_length, head_length);
        return data.data_int;
    } else if (head_length == 4) {
        wlbHead<4> data{};
        memccpy(data.data_uchar, str_data, head_length, head_length);
        return data.data_int;
    }
    assert(head_length == 0);
}







/////////////////////////////////
// WFixedBufferConnection


WFixedBufferConnection::WFixedBufferConnection(WBaseConnection::Listener *listener) :
        listener_(listener) {
}
WFixedBufferConnection::~WFixedBufferConnection() = default;

bool WFixedBufferConnection::Init(WNetWorkHandler *handler, uint32_t maxBufferSize, uint32_t messageSize) {
    this->max_buffer_size_ = maxBufferSize;
    this->message_size_    = messageSize;

    if (!recv_buffer_.Init(maxBufferSize)) {
        return false;
    }
    if (!send_buffer_.Init(maxBufferSize)) {
        return false;
    }

    this->handler_ = handler;
    assert(this->handler_);

    this->op_ |= WNetWorkHandler::OP_IN;
    this->op_ |= WNetWorkHandler::OP_ERR;
    this->op_ |= WNetWorkHandler::OP_SHUT;
    this->op_ |= WNetWorkHandler::OP_CLOS;

    this->handler_data_ = new(std::nothrow) WHandlerData(this->socket_, this);
    if (this->handler_data_ == nullptr) {
        return false;
    }

    return true;
}

bool WFixedBufferConnection::SetConnectedSocket(base_socket_type socket, const WEndPointInfo &peerInfo) {
    this->socket_ = socket;

    if (!SetTcpSocketNoDelay(this->socket_) ||
            !SetSocketKeepAlive(this->socket_)) {
        this->error_code_ = errno;
        return false;
    }
    if (!SetSocketNoBlock(this->socket_)) {
        this->error_code_ = errno;
        // error set noblock failed
    }
    if (!handler_->AddSocket(this->handler_data_, this->op_)) {
        this->error_code_ = this->handler_->GetErrorNo();
        return false;
    }

    this->peer_info_ = peerInfo;

    this->is_connected_ = true;

    return true;
}

void WFixedBufferConnection::CloseConnection() {
    if (shutdown(this->socket_, SHUT_RDWR) != 0) {
        this->error_code_ = errno;

        if (this->listener_) {
            this->listener_->OnConnectionError();
        }
    }
}

void WFixedBufferConnection::Clear() {
    this->is_connected_ = false;

    this->handler_->RemoveSocket(this->socket_);

    if (this->socket_ != -1) {
        ::close(this->socket_);
        this->socket_ = -1;
    }

    recv_buffer_.Clear();
    send_buffer_.Clear();
}

void WFixedBufferConnection::Destroy() {
    this->Clear();

    recv_buffer_.Destroy();
    send_buffer_.Destroy();

    this->listener_ = nullptr;

    delete this->handler_data_;
    this->handler_data_ = nullptr;
}

bool WFixedBufferConnection::Send(const std::string &message) {
    uint32_t msgSize    = message.size();
    uint32_t insert_len = 0;

    assert(msgSize <= this->message_size_);

    insert_len = send_buffer_.InsertMessage(message);
    if (insert_len != message.length()) {
        return false;
    }

    // 添加进 send events
    this->op_ |= WNetWorkHandler::OP_OUT;
    if (!handler_->ModifySocket(this->handler_data_, this->op_)) {
        this->error_code_ = errno;
        return false;
    }

    return true;
}

bool WFixedBufferConnection::Receive() {
    int64_t recv_len = ::recv(this->socket_,
                              this->recv_buffer_.GetRestBuffer(),
                              this->recv_buffer_.GetTopRestBufferSize(),
                              0);

    std::cout << "recv " << recv_len << std::endl;

    if (recv_len <= -1) {
        this->error_code_ = errno;
        this->listener_->OnConnectionError();
        return false;
    }
    if (recv_len == 0 && recv_buffer_.GetTopRestBufferSize() != 0) {
        this->listener_->OnConnectionClosed();
        return false;
    }

    this->recv_buffer_.UpdateWriteOffset(recv_len);

    return true;
}

void WFixedBufferConnection::HandleError(int16_t error_code) {
    this->error_code_ = error_code;
}

void WFixedBufferConnection::OnError(int16_t error_no) {
    this->is_connected_ = false;
    HandleError(error_no);

    if (this->listener_ != nullptr) {
        this->listener_->OnConnectionError();
    }

}

void WFixedBufferConnection::OnRead() {
    if (!this->Receive()) {
        return;
    }

    std::string receive_message;
    // on message
    while (true) {
        receive_message.clear();

        uint32_t len = this->recv_buffer_.GetFrontMessage(&receive_message, this->message_size_);

        if (len != this->message_size_) {
//            std::cout << "no enough message" << std::endl;
            return;
        }

        this->recv_buffer_.UpdateReadOffset(len);

        this->listener_->OnConnectionMessage(receive_message);
    }
}

void WFixedBufferConnection::OnWrite() {
    std::string send_message;
    uint32_t    msg_len  = send_buffer_.GetAllMessage(&send_message);
    ssize_t     send_len = ::send(this->socket_, send_message.c_str(), msg_len, 0);
     std::cout << "send:" << send_len << std::endl;

    this->send_buffer_.UpdateReadOffset(send_len);

    if (send_len < 0) {
        this->HandleError(errno);
        this->listener_->OnConnectionError();
        return;
    }

    if (send_buffer_.Empty()) {
        this->op_ -= WNetWorkHandler::OP_OUT;
        this->handler_->ModifySocket(this->handler_data_, this->op_);
    }

}

void WFixedBufferConnection::OnClosed() {
    this->is_connected_ = false;

    if (this->listener_ != nullptr) {
        this->listener_->OnConnectionClosed();
    }
}

void WFixedBufferConnection::OnShutdown() {
    this->is_connected_ = false;

    if (this->listener_ != nullptr) {
        this->listener_->OnConnectionShutdown();
    }
}
int WFixedBufferConnection::GetErrorCode() {
    auto e = this->error_code_;
    this->error_code_ = -1;
    return e;
}
const WEndPointInfo &WFixedBufferConnection::GetPeerInfo() {
    return this->peer_info_;
}

}

