#pragma once
#ifndef UTIL_UDP_H
#define UTIL_UDP_H


#include "Channel.h"
#include "NetWork.h"
#include "NetworkDef.h"
#include "Socket.h"
#include "Tools.h"
#include "base/Acceptor.h"
#include "base/EndPoint.h"

namespace wutils::network::ip {

class Udp {
public:
    static int Protocol() { return IPPROTO_UDP; }
    static int Type() { return SOCK_DGRAM; }

    /**
     * @tparam FAMILY IP版本 V4 or V6
     * @example
     *  Socket<V4> s1;
     * @example
     *  Socket<V6> s2;
     */
    template <class FAMILY>
    class Socket : public SOCKET {
    public:
        Socket() : SOCKET(CreateSocket<FAMILY, Udp>()){};
        Socket(const Socket &other) : SOCKET(other) {}
        ~Socket() override = default;

        Socket &operator=(const Socket &other) {
            SOCKET::operator=(other);

            return *this;
        }

        bool Bind(const typename FAMILY::EndPointInfo &info) {
            return ::bind(this->socket_.GetNativeSocket(), info.AsSockAddr(), info.GetSockAddrLen()) == 0;
        }

        int64_t Recv(const uint8_t *buffer, uint32_t buffer_len, typename FAMILY::EndPointInfo &info) {
            typename FAMILY::addr _addr;

            auto l = ::recvfrom(this->socket_, buffer, buffer_len, 0, _addr, sizeof(_addr));

            info.Assign(_addr);

            return l;
        }
        int64_t Send(const uint8_t *data, uint32_t data_len, const typename FAMILY::EndPointInfo &info) {
            return ::sendto(this->socket_, data, data_len, info.AsSockAddr(), sizeof(FAMILY::addr));
        }
    };

    /**
     * @tparam FAMILY IP版本 V4 or V6
     * @example
     *  UDPPointer<V4> u1; \n
     *  V4::EndPointInfo e1("127.0.0.1", 4000); \n
     *  u1.Start(e1, true); \n
     * @example
     *  UDPPointer<V6> u2;
     */
    // XXX: 逻辑上 UDPPointer not "is a" BaseChannel.
    template <class FAMILY>
    class UDPPointer : public BaseChannel {
    public:
        explicit UDPPointer(weak_ptr<IO_Context> handle) {
            std::cout << "UDPPointer " << std::endl;

            this->handler_             = make_shared<IO_Handle_t>();
            this->handler_->handle_    = std::move(handle);
            this->handler_->user_data_ = this;
        }
        ~UDPPointer() override {
            if(this->handler_) {
                if(this->handler_->IsEnable())
                    this->handler_->DisEnable();
            }
        }

        /**
         * @param local_endpoint    the local end point to bind
         * @param shared            address reused
         * @return                  true for everything is ok
         * @note                    bind end listen
         */
        bool Start(const typename FAMILY::EndPointInfo &local_endpoint, bool shared) {
            this->local_endpoint_ = local_endpoint;

            auto socket = MakeBindedSocket(socket_.GetNativeSocket(), shared);
            if(socket == -1) {
                return false;
            }

            this->handler_->socket_ = socket;

            this->handler_->SetEvents(event::EventType::EV_IN);
            this->handler_->Enable();
            return true;
        }

        using message_cb = std::function<void(const typename FAMILY::EndPointInfo &,
                                              const typename FAMILY::EndPointInfo &,
                                              const uint8_t *,
                                              uint32_t)>;
        message_cb                       OnMessage;
        std::function<void(SystemError)> OnError;

        // unreliable
        bool SendTo(const uint8_t *send_message, uint32_t message_len, const typename FAMILY::EndPointInfo &remote);

    private:
        void ChannelIn() final {
            typename FAMILY::EndPointInfo ei;
            uint8_t                       buf[MAX_UDP_BUFFER_LEN]{0};

            auto recv_len = this->local_endpoint_;

            if(recv_len <= 0) { // error
                onErr(SystemError::GetSysErrCode());
            } else {
                if(!OnMessage) {
                    return;
                }

                OnMessage(local_endpoint_, ei, buf, recv_len);
            }
        }
        void ChannelOut() final{};

        void onErr(SystemError);

    private:
        IO_Handle_p    handler_{nullptr};
        Socket<FAMILY> socket_;
    };
};


} // namespace wutils::network::ip


#endif // UTIL_UDP_H
