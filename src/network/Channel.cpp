#include "wutils/network/IO_Event.h"
#include <cassert>
#include <iostream>
#include <utility>

#include "wutils/network/Factory.h"

namespace wutils::network::event {


/***********************************************************
 * AcceptorChannel
 ************************************************************/
//
///***********************************************************
// * UDPPointer
// ************************************************************/
//
//
// void UDPPointer::ChannelIn() {
//    EndPointInfo ei;
//    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};
//
//    auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, ei);
//
//    if(recv_len <= 0) { // error
//        onErr(SystemError::GetSysErrCode());
//    } else {
//        if(!OnMessage) {
//            return;
//        }
//
//        OnMessage(local_endpoint_, ei, buf, recv_len);
//    }
//}
//
// void UDPPointer::onErr(SystemError err) {
//    if(err.Code() != 0) {
//        if(this->OnError) {
//            this->OnError(err);
//        }
//    }
//}
//
// bool UDPPointer::SendTo(const uint8_t *send_message, uint32_t message_len, const EndPointInfo &remote) {
//    assert(this->handler_->IsEnable());
//    assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);
//
//    auto [ip, port] = EndPointInfo::Dump(remote);
//    std::cout << "UDPPointer::SendTo " << ip << " : " << port << " " << this->handler_->socket_ << " "
//              << " msg:" << send_message << " size " << (size_t)message_len << std::endl;
//
//    auto len = ::sendto(
//            this->handler_->socket_, (void *)send_message, message_len, 0, remote.GetAddr(), remote.GetSockSize());
//    if(len == -1) {
//        std::cout << "send to err " << std::endl;
//        this->onErr(SystemError::GetSysErrCode());
//        return false;
//    }
//
//    return true;
//}
//
//
///***********************************************************
// * UDPChannel
// ************************************************************/
//
// UDPChannel::UDPChannel(std::weak_ptr<ev_hdle_t> handle) {
//    this->handler_             = make_shared<ev_hdler_t>();
//    this->handler_->user_data_ = this;
//    this->handler_->handle_    = std::move(handle);
//}
//
// UDPChannel::~UDPChannel() {
//    if(this->handler_) {
//        if(this->handler_->IsEnable())
//            this->handler_->DisEnable();
//    }
//}
//
// bool UDPChannel::Start(const EndPointInfo &local_ep, const EndPointInfo &remote_ep, bool shared) {
//
//    this->local_endpoint_  = local_ep;
//    this->remote_endpoint_ = remote_ep;
//
//    auto socket = MakeBindedSocket(local_endpoint_, shared);
//    if(socket == -1) {
//        return false;
//    }
//
//
//    bool ok = ConnectToHost(socket, remote_endpoint_);
//    // may not be fail
//    // if(!ok) {
//    //     assert("UDPChannel ConnectToHost err ");
//    //     std::cout << "UDPChannel ConnectToHost " << socket << std::endl;
//    // }
//
//    this->handler_->socket_ = socket;
//    this->handler_->SetEvents(HandlerEventType::EV_IN);
//    this->handler_->Enable();
//
//    return true;
//}
//
// void UDPChannel::EventIn() {
//    std::cout << "UDPChannel channel in" << std::endl;
//
//    EndPointInfo ei;
//    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};
//
//    // auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);
//    auto recv_len = recv(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, 0);
//
//    // if(ei != this->remote_endpoint_) {
//    //     std::cout << "not wanted remote " << std::endl;
//    //     return;
//    // }
//
//    if(recv_len <= 0) { // error
//        onErr(SystemError::GetSysErrCode());
//    } else {
//        if(listener_.expired()) {
//            return;
//        }
//
//        listener_.lock()->OnMessage(buf, recv_len);
//    }
//}
//
// void UDPChannel::onErr(SystemError err) {
//    if(err.Code() != 0) {
//        if(!listener_.expired()) {
//            listener_.lock()->OnError(err);
//        }
//    }
//}
//
// bool UDPChannel::Send(const uint8_t *send_message, uint32_t message_len) {
//    assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);
//
//    // auto len = ::sendto(this->handler_->socket_,
//    //                     (void *)send_message,
//    //                     message_len,
//    //                     0,
//    //                     remote_endpoint_.GetAddr(),
//    //                     remote_endpoint_.GetSockSize());
//    auto len = send(this->handler_->socket_, send_message, message_len, 0);
//
//    if(len == -1) {
//        std::cout << "send to err " << std::endl;
//        this->onErr(SystemError::GetSysErrCode());
//        return false;
//    }
//
//    return true;
//}

//
///***********************************************************
// * ASChannel
// ************************************************************/
//
// ASChannel::ASChannel(const EndPointInfo &local, const EndPointInfo &remote, ev_hdler_p h) :
//    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {
//
//    assert(this->event_handler_);
//
//    this->event_handler_->user_data_ = this;
//
//    auto [ip, port] = EndPointInfo::Dump(this->remote_endpoint_);
//    std::cout << "ASChannel " << ip << port << std::endl;
//}
// ASChannel::~ASChannel() {
//    this->ShutDown(SHUT_RDWR);
//
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
//    this->event_handler_->SetEvents(this->event_handler_->GetEvents() | HandlerEventType::EV_IN);
//    if(!this->event_handler_->IsEnable())
//        this->event_handler_->Enable();
//}
// void ASChannel::ASend(ASChannel::ABuffer buffer) {
//    //    std::cout << "ASChannel ASend " << std::endl;
//    assert(this->event_handler_);
//
//    send_buffer_ = buffer;
//
//    this->event_handler_->SetEvents(this->event_handler_->GetEvents() | HandlerEventType::EV_OUT);
//    if(!this->event_handler_->IsEnable())
//        this->event_handler_->Enable();
//}
//
// void ASChannel::EventIn() {
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
//    this->event_handler_->SetEvents(this->event_handler_->GetEvents() & (~HandlerEventType::EV_IN));
//
//    recv_buffer_.buf_len = len;
//
//    if(!this->listener_.expired()) {
//        this->listener_.lock()->onReceive(recv_buffer_);
//    }
//}
// void ASChannel::EventOut() {
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
//        this->event_handler_->SetEvents(this->event_handler_->GetEvents() & (~HandlerEventType::EV_OUT));
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


} // namespace wutils::network::event
