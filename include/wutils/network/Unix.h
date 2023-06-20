#pragma once
#ifndef UTIL_UNIX_H
#define UTIL_UNIX_H

#include <array>
#include <cstdint>
#include <unistd.h>     // close(int)

#include <netinet/in.h> // AF_UNIX SOCK_DGRAM SOCK_STREAM

#include <sys/socket.h>
#include <sys/un.h>

#include "Socket.h"

namespace wutils::network {

class Unix {
    static int Family() {
        return AF_UNIX;
        // return AF_LOCAL; // both is ok
    }
    using addr                    = ::sockaddr_un;
    static constexpr int addr_len = sizeof(addr);
    using Addr                    = std::array<uint8_t, addr_len>;

    class STREAM {
        static int Type() { return SOCK_STREAM; }
        static int Protocol() { return 0; }

        class Socket : public SOCKET {};
    };
    class DGRAM {
        static int Type() { return SOCK_DGRAM; }
        static int Protocol() { return 0; }

        class Socket : public SOCKET {};
    };
    class SEQPACKET {
        static int Type() { return SOCK_SEQPACKET; }
        static int Protocol() { return 0; }

        class Socket : public SOCKET {};
    };
};

} // namespace wutils::network

#endif // UTIL_UNIX_H
