#pragma once
#ifndef UTIL_ACCEPTOR_H
#define UTIL_ACCEPTOR_H

#include "IO_Context.h"

namespace wutils::network::event {


template <class FAMILY, class PROTOCOL>
class Acceptor : public IOInOnly {
public:
    explicit Acceptor(weak_ptr<IO_Context> context) {
        this->handler_           = make_shared<IO_Context::IO_Handle>();
        this->handler_->socket_  = this->acceptor_;
        this->handler_->handle_  = std::move(context);
        this->handler_->listener = this;
    }
    ~Acceptor() override {
        if(this->handler_) {
            if(this->handler_->IsEnable())
                this->handler_->DisEnable();
        }
        this->acceptor_.Close();
    }

    using Family       = FAMILY;
    using Protocol     = PROTOCOL;
    using acceptor     = typename Protocol::template Acceptor<Family>;
    using EndPointInfo = typename Family::EndPointInfo;

    bool Bind(const EndPointInfo &info) { return acceptor_.Bind(info); }
    bool Listen() { return acceptor_.Listen(); }

    bool IsNonBlock() { return acceptor_.IsNonBlock(); }
    bool SetNonBlock() { return acceptor_.SetNonBlock(); }
    bool SetReusePort() { return acceptor_.SetReusePort(); }

    using accept_cb = std::function<void(const EndPointInfo &, const EndPointInfo &, IO_Context::IO_Handle_p)>;
    using error_cb  = std::function<void(SystemError)>;
    accept_cb OnAccept;
    error_cb  OnError;

private:
    void EventIn() final {
        assert(this->handler_);

        EndPointInfo ei;

        auto cli = this->acceptor_.Accept(ei);

        if(!cli) { // error
            if(OnError) {
                OnError(SystemError::GetSysErrCode());
            }
        } else {
            if(!OnAccept) {
                return;
            }

            auto handler     = make_shared<IO_Context::IO_Handle>();
            handler->socket_ = cli;
            handler->handle_ = this->handler_->handle_;

            OnAccept(this->acceptor_.GetLocalInfo(), ei, handler);
        }
    }

private:
    IO_Context::IO_Handle_p handler_{nullptr};
    acceptor                acceptor_;
    EndPointInfo            local_endpoint_;
};

} // namespace wutils::network::event

#endif // UTIL_ACCEPTOR_H
