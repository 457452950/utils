#pragma once
#ifndef UTIL_UNIX_H
#define UTIL_UNIX_H

#include <array>
#include <cstdint>
#include <filesystem>
#include <unistd.h>     // close(int)

#include <netinet/in.h> // AF_UNIX SOCK_DGRAM SOCK_STREAM

#include <sys/socket.h>
#include <sys/un.h>

#include "Tools.h"
#include "base/EndPoint.h"
#include "base/IAcceptor.h"
#include "wutils/network/base/ISocket.h"

namespace wutils::network {

class Unix {
public:
    static int Family() {
        return AF_UNIX;
        // return AF_LOCAL; // both is ok
    }

    using sockaddr_t                  = ::sockaddr_un;
    static constexpr int sockaddr_len = sizeof(sockaddr_t);

    static bool GetSockaddr_un(const std::string &path, bool create_file, sockaddr_t *sockaddr) {
        std::filesystem::path _path(path);
        if(!_path.is_absolute()) {
            return false;
        }

        auto len = path.size();
        if(len > (108 + (int)(!create_file))) {
            return false;
        }

        sockaddr->sun_family = Unix::Family();
        int s                = 0;
        if(!create_file) {
            ++s;
            sockaddr->sun_path[0] = '\0';
        }
        memcpy(sockaddr->sun_path + s, path.data(), path.size());
        return true;
    }

    class EndPointInfo : public IEndPointInfo<Unix> {
    public:
        EndPointInfo() : IEndPointInfo<Unix>() {}
        explicit EndPointInfo(const std::string &path) : IEndPointInfo<Unix>() { this->is_valid_ = this->Assign(path); }

        using IEndPointInfo<Unix>::Assign;
        bool Assign(const std::string &path) {
            is_valid_ = GetSockaddr_un(path, false, (sockaddr_t *)&this->sockaddr_);
            return is_valid_;
        }

        std::tuple<std::string, bool> Dump() {
            auto        native_struct = (sockaddr_t *)this->sockaddr_.data();
            std::string path(native_struct->sun_path + 1);
            return {path, false};
        }
    };

    class Stream {
    public:
        static int Type() { return SOCK_STREAM; }
        static int Protocol() { return 0; }

        class Socket : public ISocket {
        public:
            Socket() : ISocket(CreateSocket<Unix, Stream>()){};
            explicit Socket(std::nullptr_t p) : ISocket(p) {}
            explicit Socket(socket_t s) : ISocket(s) {}
            Socket(const Socket &other) : ISocket(other) {}
            ~Socket() = default;

            Socket &operator=(const Socket &other) {
                ISocket::operator=(other);
                return *this;
            }

            bool Bind(const EndPointInfo &info) {
                return ::bind(this->socket_, (sockaddr *)info.AsSockAddr(), info.GetSockAddrLen()) == 0;
            }

            // TODO: connect, shutdown

            int64_t Recv(uint8_t *buffer, uint32_t buffer_len) { return ::recv(this->socket_, buffer, buffer_len, 0); }
            int64_t Send(const uint8_t *data, uint32_t data_len) { return ::send(this->socket_, data, data_len, 0); }
            bool    Connect(const EndPointInfo &info) {
                return ::connect(this->socket_, (sockaddr *)info.AsSockAddr(), info.GetSockAddrLen()) == 0;
            }
            // void ShutDown(ShutType how) { ::shutdown(this->socket_, how); }
        };

        using Acceptor = IAcceptor<Unix, Stream, Socket>;
    };
    class Dgram {
        static int Type() { return SOCK_DGRAM; }
        static int Protocol() { return 0; }

        class Socket : public ISocket {};
    };
    class Seqpacket {
        static int Type() { return SOCK_SEQPACKET; }
        static int Protocol() { return 0; }

        class Socket : public ISocket {};
    };
};

} // namespace wutils::network

#endif // UTIL_UNIX_H
