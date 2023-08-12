#include <gtest/gtest.h>

#include "wutils/net/dns/DNS.h"

TEST(dns, dns) {
    using namespace wutils::net;
    auto future = dns::DNS::Request("www.baidu.com");

    auto res = future.get();
    GTEST_LOG_(INFO) << res.Success();
    for(auto item : res.Get()) {
        auto [ip, port] = item.net_address.Dump();
        GTEST_LOG_(INFO) << item.protocol << " " << item.socket_type << " " << item.name << " " << ip << " " << port
                         << std::endl;
    }
    GTEST_LOG_(INFO) << res.Get().size();

    future = dns::DNS::Request("www.baidu.com", dns::BOTH, dns::TCP);
    res    = future.get();
    GTEST_LOG_(INFO) << res.Success();
    for(auto item : res.Get()) {
        auto [ip, port] = item.net_address.Dump();
        GTEST_LOG_(INFO) << item.protocol << " " << item.socket_type << " " << item.name << " " << ip << " " << port
                         << std::endl;
    }
    GTEST_LOG_(INFO) << res.Get().size();

    future = dns::DNS::Request("www.baidu.com", dns::INET6);
    res    = future.get();
    GTEST_LOG_(INFO) << res.Success();
    for(auto item : res.Get()) {
        auto [ip, port] = item.net_address.Dump();
        GTEST_LOG_(INFO) << item.protocol << " " << item.socket_type << " " << item.name << " " << ip << " " << port
                         << std::endl;
    }
    GTEST_LOG_(INFO) << res.Get().size();

    future = dns::DNS::Request("www.baidududu.com");
    res    = future.get();
    GTEST_LOG_(INFO) << res.Success();
    if(!res.Success()) {
        GTEST_LOG_(INFO) << res.GetError().message();
    } else {
        for(const auto &item : res.Get()) {
            auto [ip, port] = item.net_address.Dump();
            GTEST_LOG_(INFO) << item.protocol << " " << item.socket_type << " " << item.name << " " << ip << " " << port
                             << std::endl;
        }
    }
    GTEST_LOG_(INFO) << res.Get().size();

    future = dns::DNS::Request("www.google.com");
    res    = future.get();
    GTEST_LOG_(INFO) << res.Success();
    if(!res.Success()) {
        GTEST_LOG_(INFO) << res.GetError().message();
    } else {
        for(const auto &item : res.Get()) {
            auto [ip, port] = item.net_address.Dump();
            GTEST_LOG_(INFO) << item.protocol << " " << item.socket_type << " " << item.name << " " << ip << " " << port
                             << std::endl;
        }
    }
    GTEST_LOG_(INFO) << res.Get().size();
}
