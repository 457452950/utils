#include "wutils/network/io_event/IOEvent.h"
#include <cassert>
#include <iostream>

namespace wutils::network {

///***********************************************************
// * Channel
// ************************************************************/
//
// Channel::Channel(const EndPoint &local, const EndPoint &remote, io_hdle_p h) :
//    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {
//
//    assert(this->event_handler_);
//
//    this->event_handler_->user_data_ = this;
//    this->event_handler_->SetEvents(EventType::EV_IN);
//    this->event_handler_->Enable();
//
//    auto [ip, port] = EndPoint::Dump(this->remote_endpoint_);
//    std::cout << "Channel " << ip << port << std::endl;
//}
//
// Channel::~Channel() {
//    this->ShutDown(SHUT_RDWR);
//
//    if(this->event_handler_) {
//        if(this->event_handler_->IsEnable())
//            this->event_handler_->DisEnable();
//    }
//}
//
// bool Channel::Init() { return true; }
//
// void Channel::ShutDown(int how) {
//    // std::cout << "Channel::ShutDown()" << std::endl;
//
//    assert(how >= SHUT_RD);
//    assert(how <= SHUT_RDWR);
//
//    ::shutdown(this->event_handler_->socket_, how);
//}
//
// void Channel::Send(const uint8_t *send_message, uint32_t message_len) {
//    assert(message_len <= MAX_CHANNEL_SEND_SIZE);
//
//    auto s_buf = this->send_buf_;
//
//    // has buf
//    if(max_send_buf_size_ != 0 && !s_buf->IsEmpty() > 0) {
//        // push data to buf
//        auto l = s_buf->WriteSome(send_message, message_len);
//        if(l != message_len) {
//            abort();
//            return;
//        }
//
//        std::cout << "save all " << message_len << std::endl;
//
//        auto events = this->event_handler_->GetEvents();
//        events |= (EventType::EV_OUT);
//        this->event_handler_->SetEvents(events);
//        return;
//    }
//
//    // try to send
//    auto res = ::send(this->event_handler_->socket_, send_message, message_len, 0);
//    if(res < 0) {
//        auto err = SystemError::GetSysErrCode();
//        if(err == EAGAIN || err == EWOULDBLOCK) {
//        } else {
//            this->onChannelError(err);
//        }
//        return;
//    }
//
//    if(res < message_len) {
//        assert(max_send_buf_size_ != 0);
//
//        auto l = s_buf->WriteSome(send_message + res, message_len - (uint32_t)res);
//        assert(l == (message_len - res));
//
//        auto events = this->event_handler_->GetEvents();
//        events |= (EventType::EV_OUT);
//        this->event_handler_->SetEvents(events);
//    } else {
//        //        std::cout << "send all " << message_len << std::endl;
//    }
//}
//
// void Channel::SetRecvBufferMaxSize(uint64_t max_size) {
//    this->max_recv_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_RECV_BUFFER_SIZE);
//
//    recv_buf_->Init(this->max_recv_buf_size_);
//}
//
// void Channel::SetSendBufferMaxSize(uint64_t max_size) {
//    this->max_send_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_SEND_BUFFER_SIZE);
//
//    send_buf_->Init(this->max_send_buf_size_);
//}
//
// void Channel::IOIn() {
//    //    std::cout << "Channel channel in" << std::endl;
//
//    assert(this->event_handler_);
//    assert(this->max_recv_buf_size_);
//    assert(this->recv_buf_->GetWriteableBytes() != 0);
//    assert(!this->listener_.expired());
//
//    auto r_buff = recv_buf_;
//
//    int64_t recv_len = ::recv(this->event_handler_->socket_, r_buff->PeekWrite(), r_buff->GetWriteableBytes(), 0);
//
//    //    std::cout << "Channel channel in recv " << recv_len << std::endl;
//
//    if(recv_len == 0) { // has emitted in recv_buf_->WriteSome
//        this->onChannelClose();
//        return;
//    } else if(recv_len == -1) {
//        auto eno = SystemError::GetSysErrCode();
//        if(eno == ECONNRESET) {
//            this->onChannelClose();
//            return;
//        }
//        if(eno == EAGAIN || eno == EWOULDBLOCK) {
//
//        } else {
//            this->onChannelError(eno);
//        }
//        return;
//    }
//
//    r_buff->UpdateWriteBytes(recv_len);
//
//    r_buff->ReadUntil([&](const uint8_t *msg, uint32_t len) -> int64_t {
//        this->listener_.lock()->onReceive(msg, len);
//        return len;
//    });
//
//    //    while(!this->recv_buf_->IsEmpty()) {
//    //        this->listener_.lock()->onReceive(this->recv_buf_->ConstPeekRead(),
//    this->recv_buf_->GetReadableBytes());
//    //        this->recv_buf_->SkipReadBytes(this->recv_buf_->GetReadableBytes());
//    //    }
//}
//
//// can write
// void Channel::IOOut() {
//     std::cout << "Channel channel out" << std::endl;
//     assert(this->event_handler_);
//
//     auto s_buf = this->send_buf_;
//
//     // 无缓冲设计或无缓冲数据
//     if(this->max_send_buf_size_ == 0 || s_buf->IsEmpty()) {
//         auto events = this->event_handler_->GetEvents();
//         events &= (~EventType::EV_OUT);
//         this->event_handler_->SetEvents(events);
//         return;
//     }
//
//     // 发送缓冲数据
//     while(!s_buf->IsEmpty()) {
//         auto send_len = ::send(this->event_handler_->socket_, s_buf->PeekRead(), s_buf->GetReadableBytes(), 0);
//         if(send_len == 0) {
//             this->onChannelClose();
//             return;
//         }
//         if(send_len < 0) {
//             auto eno = SystemError::GetSysErrCode();
//             if(eno == EAGAIN || eno == EWOULDBLOCK) {
//                 break;
//             }
//             if(eno == ECONNRESET) {
//                 this->onChannelClose();
//             } else {
//                 this->onChannelError(eno);
//             }
//             return;
//         }
//
//         s_buf->SkipReadBytes(send_len);
//     }
//
//     // 无缓冲数据时，停止监听 out 事件
//     if(s_buf->IsEmpty()) {
//         auto events = this->event_handler_->GetEvents();
//         events      = events & (~EventType::EV_OUT);
//         this->event_handler_->SetEvents(events);
//     }
// }
//
// void Channel::onChannelClose() {
//     if(!this->listener_.expired())
//         this->listener_.lock()->onChannelDisConnect();
// }
//
// void Channel::onChannelError(SystemError error) {
//     std::cout << "error " << error << std::endl;
//     if(!this->listener_.expired())
//         this->listener_.lock()->onError(error);
// }
//
//
///***********************************************************
// * ASChannel
// ************************************************************/
//
// ASChannel::ASChannel(const EndPoint &local, const EndPoint &remote, io_hdle_p h) :
//    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {
//
//    assert(this->event_handler_);
//
//    this->event_handler_->user_data_ = this;
//
//    auto [ip, port] = EndPoint::Dump(this->remote_endpoint_);
//    std::cout << "ASChannel " << ip << port << " " << this->event_handler_.get() << std::endl;
//}
// ASChannel::~ASChannel() {
//    std::cout << "!ASChannel()" << this->event_handler_.get() << std::endl;
//    this->ShutDown(SHUT_RDWR);
//    if(this->event_handler_) {
//        if(this->event_handler_->IsEnable())
//            this->event_handler_->DisEnable();
//    }
//}
// void ASChannel::ShutDown(int how) {
//    // std::cout << "Channel::ShutDown()" << std::endl;
//
//    assert(how >= SHUT_RD);
//    assert(how <= SHUT_RDWR);
//
//    ::shutdown(this->event_handler_->socket_, how);
//}
// void ASChannel::ARecv(ASChannel::ABuffer buffer) {
//    //    std::cout << "ASChannel ARecv " << std::endl;
//    assert(this->event_handler_);
//
//    recv_buffer_ = buffer;
//
//    this->event_handler_->SetEvents(this->event_handler_->GetEvents() | EventType::EV_IN);
//    if(!this->event_handler_->IsEnable())
//        this->event_handler_->Enable();
//}
// void ASChannel::ASend(ASChannel::ABuffer buffer) {
//    //    std::cout << "ASChannel ASend " << std::endl;
//    assert(this->event_handler_);
//
//    send_buffer_ = buffer;
//
//    auto len = ::send(this->event_handler_->socket_, send_buffer_.buffer, send_buffer_.buf_len, 0);
//    if(len != buffer.buf_len) {
//        send_buffer_.buffer += len;
//        send_buffer_.buf_len -= len;
//    } else {
//        if(!this->listener_.expired()) {
//            this->listener_.lock()->onSend(send_buffer_);
//        }
//        return;
//    }
//
//    this->event_handler_->SetEvents(this->event_handler_->GetEvents() | EventType::EV_OUT);
//    if(!this->event_handler_->IsEnable())
//        this->event_handler_->Enable();
//}
//
// void ASChannel::IOIn() {
//    //    std::cout << "ASChannel Channel in" << std::endl;
//    auto len = ::recv(this->event_handler_->socket_, recv_buffer_.buffer, recv_buffer_.buf_len, 0);
//
//    if(len == 0 & recv_buffer_.buf_len != 0) {
//        this->onChannelClose();
//        return;
//    }
//    if(len < 0) {
//        auto eno = SystemError::GetSysErrCode();
//        if(eno == ECONNRESET) {
//            this->onChannelClose();
//            return;
//        }
//        this->onChannelError(eno);
//        return;
//    }
//
//    //    this->event_handler_->SetEvents(this->event_handler_->GetEvents() & (~EventType::EV_IN));
//
//    recv_buffer_.buf_len = len;
//
//    if(!this->listener_.expired()) {
//        this->listener_.lock()->onReceive(recv_buffer_);
//    }
//}
// void ASChannel::IOOut() {
//    auto len = ::send(this->event_handler_->socket_, send_buffer_.buffer, send_buffer_.buf_len, 0);
//    if(len == 0 && send_buffer_.buf_len != 0) {
//        this->onChannelClose();
//        return;
//    }
//    if(len < 0) {
//        auto eno = SystemError::GetSysErrCode();
//        if(eno == ECONNRESET) {
//            this->onChannelClose();
//            return;
//        }
//        this->onChannelError(eno);
//        return;
//    }
//
//    if(len == this->send_buffer_.buf_len) {
//        this->event_handler_->SetEvents(this->event_handler_->GetEvents() & (~EventType::EV_OUT));
//
//        if(!this->listener_.expired()) {
//            this->listener_.lock()->onSend(send_buffer_);
//        }
//    } else {
//        send_buffer_.buffer = send_buffer_.buffer + len;
//        send_buffer_.buf_len -= len;
//    }
//}
// void ASChannel::onChannelClose() {
//    if(!this->listener_.expired())
//        this->listener_.lock()->onChannelDisConnect();
//}
// void ASChannel::onChannelError(SystemError error) {
//    std::cout << "error " << error << std::endl;
//    if(!this->listener_.expired())
//        this->listener_.lock()->onError(error);
//}


} // namespace wutils::network
