#pragma once
#ifndef UTIL_NETADDRESS_H
#define UTIL_NETADDRESS_H

#include <cassert>
#include <cstring> // memcpy
#include <iostream>

#include "base/Native.h"
#include "wutils/SharedPtr.h"

namespace wutils::network {

// sockaddr + family + hash
struct NetAddress {
    NetAddress() = default;
    NetAddress(const NetAddress &other) : family_(other.family_), hash(other.hash) {
        this->data_ = make_unique<uint8_t[]>(other.GetSockSize());
        std::copy(other.data_.get(), other.data_.get() + other.GetSockSize(), this->data_.get());
    }
    NetAddress(NetAddress &&other) noexcept : family_(other.family_), data_(std::move(other.data_)), hash(other.hash) {}
    ~NetAddress() = default;

    NetAddress &operator=(const NetAddress &other) {
        if(this == &other) {
            return *this;
        }

        family_ = other.family_;
        hash    = other.hash;

        this->data_ = make_unique<uint8_t[]>(other.GetSockSize());
        std::copy(other.data_.get(), other.data_.get() + other.GetSockSize(), this->data_.get());

        return *this;
    }

    static NetAddress *Make(const std::string &address, uint16_t port, AF_FAMILY family) {
        auto ep = new NetAddress();

        if(ep->Assign(address, port, family)) {
            return ep;
        }

        delete ep;
        return nullptr;
    }

    bool Assign(const std::string &address, uint16_t port, AF_FAMILY family) {
        this->family_ = family;
        this->data_   = make_unique<uint8_t[]>(GetSockSize());

        switch(this->family_) {
        case AF_FAMILY::INET:
            if(!MakeSockAddr_in(address, port, (sockaddr_in *)AsSockAddr())) {
                return false;
            }
            break;
        case AF_FAMILY::INET6:
            if(!MakeSockAddr_in6(address, port, (sockaddr_in6 *)AsSockAddr())) {
                return false;
            }
            break;
        default:
            // never achieve
            abort();
        }

        this->SetHash();
        return true;
    };
    bool Assign(const sockaddr *sock, AF_FAMILY family) {
        this->family_ = family;
        this->data_   = make_unique<uint8_t[]>(GetSockSize());

        switch(this->family_) {
        case AF_INET:
            memcpy(this->data_.get(), sock, sizeof(sockaddr_in));
            break;
        case AF_INET6:
            memcpy(this->data_.get(), sock, sizeof(sockaddr_in6));
        default:
            assert("never acheive");
            break;
        }

        this->SetHash();
        return true;
    };
    bool Assign(const sockaddr *sock, socklen_t socklen) {
        switch(socklen) {
        case v4::SOCKADDR_LEN:
            return this->Assign(sock, v4::FAMILY);
        case v6::SOCKADDR_LEN:
            return this->Assign(sock, v6::FAMILY);
        default:
            abort();
        }
        abort();
        return false;
    }

    static NetAddress *Emplace(const sockaddr *addr, AF_FAMILY family) {
        auto info     = new NetAddress();
        info->family_ = family;
        info->data_   = make_unique<uint8_t[]>(info->GetSockSize());

        switch(info->family_) {
        case v4::FAMILY:
            memcpy(info->data_.get(), addr, v4::SOCKADDR_LEN);
            break;
        case v6::FAMILY:
            memcpy(info->data_.get(), addr, v6::SOCKADDR_LEN);
        default:
            assert("never achieve");
            break;
        }

        info->SetHash();
        return info;
    }

    const sockaddr *AsSockAddr() const { return (sockaddr *)data_.get(); }
    sockaddr       *AsSockAddr() { return (sockaddr *)data_.get(); }
    AF_FAMILY       GetFamily() const { return family_; }
    unsigned long   GetSockSize() const { return family_ == v4::FAMILY ? v4::SOCKADDR_LEN : v6::SOCKADDR_LEN; }

    std::tuple<std::string, uint16_t> Dump() const {
        std::string s;
        uint16_t    p{0};

        do {
            if(this->family_ == v4::FAMILY) {
                auto *SockAddrIn = reinterpret_cast<const struct sockaddr_in *>(this->data_.get());

                if(!IpAddrToString(SockAddrIn->sin_addr, s)) {
                    break;
                }
                if(!NtoHS(SockAddrIn->sin_port, &p)) {
                    break;
                }

            } else if(this->family_ == v6::FAMILY) {
                auto *sockAddrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(this->data_.get());

                if(!IpAddrToString(sockAddrIn6->sin6_addr, s)) {
                    break;
                }
                if(!NtoHS(sockAddrIn6->sin6_port, &p)) {
                    break;
                }
            }
        } while(false);

        return {s, p};
    }

private:
    AF_FAMILY             family_{AF_FAMILY::INET};
    unique_ptr<uint8_t[]> data_{nullptr};

private:
    /*
             * Hash for IPv4
             *
             0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |              PORT             |             IP                |
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |              IP               |                           |F|P|
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *
             * Hash for IPv6
             *
             0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |              PORT             | IP[0] ^  IP[1] ^ IP[2] ^ IP[3]|
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             |IP[0] ^  IP[1] ^ IP[2] ^ IP[3] |          IP[0] >> 16      |F|P|
             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             */
    void SetHash() {

        switch(this->family_) {
        case AF_INET: {
            auto *SockAddrIn = reinterpret_cast<const struct sockaddr_in *>(this->data_.get());

            const uint64_t address = ntohl(SockAddrIn->sin_addr.s_addr);
            const uint64_t port    = (ntohs(SockAddrIn->sin_port));

            this->hash  = port << 48;
            this->hash |= address << 16;
            this->hash |= 0x0000; // AF_INET.

            break;
        }
        case AF_INET6: {
            auto *sockAddrIn6 = reinterpret_cast<const struct sockaddr_in6 *>(this->data_.get());
            auto *a           = reinterpret_cast<const uint32_t *>(std::addressof(sockAddrIn6->sin6_addr));

            const auto     address1 = a[0] ^ a[1] ^ a[2] ^ a[3];
            const auto     address2 = a[0];
            const uint64_t port     = ntohs(sockAddrIn6->sin6_port);

            this->hash  = port << 48;
            this->hash |= static_cast<uint64_t>(address1) << 16;
            this->hash |= address2 >> 16 & 0xFFFC;
            this->hash |= 0x0002; // AF_INET6.

            break;
        }
        case UNIX:
        default:
            // never achieve
            abort();
        }

        // note:no safed protocol
        // Override least significant bit with protocol information:
        // - If UDPPointer, start with 0.
        // - If TCP, start with 1.
        // if (this->protocol == AF_PROTOL::UDPPointer)
        // {
        // 	this->hash |= 0x0000;
        // }
        // else
        // {
        // 	this->hash |= 0x0001;
        // }
    }

public:
    uint64_t hash{0u};
    bool     operator==(const NetAddress &other) const noexcept { return this->hash == other.hash; }
    bool     operator!=(const NetAddress &other) const noexcept { return this->hash != other.hash; }
};

} // namespace wutils::network


namespace std {
template <>
class hash<wutils::network::NetAddress> {
public:
    size_t operator()(const wutils::network::NetAddress &it) const noexcept { return it.hash; }
};
} // namespace std


#endif // UTIL_NETADDRESS_H
