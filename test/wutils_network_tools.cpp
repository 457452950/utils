#include <gtest/gtest.h>

#include "wutils/Error.h"
#include "wutils/network/Ip.h"
#include "wutils/network/TCP.h"
#include "wutils/network/Tools.h"
#include "wutils/network/UNIX.h"

#include "wutils/Defer.h"


TEST(TOOLS, 端口网络序字节序转换) {
    using namespace wutils::network;
    uint16_t h      = 4000;
    uint16_t n      = 0;
    uint16_t answer = 0;

    HtoNS(h, &n);
    NtoHS(n, &answer);

    ASSERT_EQ(h, answer);
}

TEST(TOOLS, 端口网络序字节序转换异常) {
    using namespace wutils::network;
    uint16_t h = 4000;
    uint16_t n = 0;

    ASSERT_NO_THROW(HtoNS(0, &n));
    ASSERT_NO_THROW(NtoHS(n, &h));
    ASSERT_EQ(h, 0);

    ASSERT_NO_THROW(HtoNS(-1, &n));
    ASSERT_NO_THROW(NtoHS(n, &h));
    ASSERT_EQ(h, UINT16_MAX);

    ASSERT_NO_THROW(HtoNS(UINT16_MAX, &n));
    ASSERT_NO_THROW(NtoHS(n, &h));
    ASSERT_EQ(h, UINT16_MAX);

    ASSERT_NO_THROW(NtoHS(0, &h));
    ASSERT_NO_THROW(NtoHS(-1, &h));
    ASSERT_NO_THROW(NtoHS(UINT16_MAX, &h));

    ASSERT_NO_THROW(HtoNS(UINT32_MAX, &n));
    ASSERT_NO_THROW(HtoNS(UINT64_MAX, &n));
    ASSERT_NO_THROW(NtoHS(UINT32_MAX, &h));
    ASSERT_NO_THROW(NtoHS(UINT64_MAX, &h));
}

TEST(TOOLS, IPv4数据结构转换) {
    using namespace wutils::network;
    ::in_addr   addr{};
    std::string ip;

    ASSERT_TRUE(ip::IsValidIp4("127.0.0.1"));
    ASSERT_TRUE(ip::IsValidIp4("168.127.56.1"));
    ASSERT_TRUE(ip::IsValidIp4("192.168.101.1"));
    ASSERT_FALSE(ip::IsValidIp4("::1"));
    ASSERT_FALSE(ip::IsValidIp4("1.0.0.256"));
    ASSERT_FALSE(ip::IsValidIp4("256.0.0.1"));
    ASSERT_FALSE(ip::IsValidIp4("...."));


    ASSERT_TRUE(ip::IpStrToAddr("127.0.0.1", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("127.0.0.1"));

    ASSERT_TRUE(ip::IpStrToAddr("168.127.56.1", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("168.127.56.1"));

    ASSERT_TRUE(ip::IpStrToAddr("192.168.101.1", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("192.168.101.1"));

    ASSERT_ANY_THROW(ip::IpStrToAddr(nullptr, &addr));
    ASSERT_FALSE(ip::IpStrToAddr("::1", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("256.0.0.1", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("1.0.0.256", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("001.168.101.1", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("....", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("....", (in_addr *)nullptr));
}

TEST(TOOLS, IPv6数据结构转换) {
    using namespace wutils::network;
    ::in6_addr  addr{};
    std::string ip;

    ASSERT_TRUE(ip::IpStrToAddr("::1", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("::1"));

    ASSERT_TRUE(ip::IpStrToAddr("::2", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("::2"));

    ASSERT_TRUE(ip::IpStrToAddr("fe80::bb5c:37cb:58fb:5519", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("fe80::bb5c:37cb:58fb:5519"));

    ASSERT_TRUE(ip::IpStrToAddr("2408:8256:f183:2d5c:1c99:42c:a0fd:a7b1", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("2408:8256:f183:2d5c:1c99:42c:a0fd:a7b1"));

    ASSERT_TRUE(ip::IpStrToAddr("2408:8256:f183:2d5c:f820:a941:ddac:4", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("2408:8256:f183:2d5c:f820:a941:ddac:4"));

    ASSERT_TRUE(ip::IpStrToAddr("2408:8256:f183:2d5c:79a1:3867:6acb:358b", &addr));
    ASSERT_TRUE(ip::IpAddrToStr(addr, ip));
    ASSERT_EQ(ip, std::string("2408:8256:f183:2d5c:79a1:3867:6acb:358b"));

    ASSERT_FALSE(ip::IpStrToAddr("127.0.0.1", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("256.0.0.1", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("1.0.0.256", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("....", &addr));
    ASSERT_FALSE(ip::IpStrToAddr("....", (in6_addr *)nullptr));
}

TEST(TOOLS, v4_Address) {
    using namespace wutils::network;
    ip::v4::Address e1;
    ip::v4::Address e2;
    ASSERT_FALSE(e1);
    ASSERT_FALSE(e1.Assign("::1"));
    ASSERT_TRUE(e1.Assign("127.0.0.1"));
    ASSERT_EQ(e1.AsString(), "127.0.0.1");
    ASSERT_TRUE(e1);

    auto data = e1.AsInAddr();
    e2.Assign(data);
    ASSERT_EQ(e2.AsString(), "127.0.0.1");
    ASSERT_TRUE(e2);
}

TEST(TOOLS, v6_Address) {
    using namespace wutils::network;
    ip::v6::Address e1;
    ip::v6::Address e2;
    ASSERT_FALSE(e1);
    ASSERT_FALSE(e1.Assign("127.0.0.1"));
    ASSERT_TRUE(e1.Assign("2408:8256:f183:2d5c:79a1:3867:6acb:358b"));
    ASSERT_EQ(e1.AsString(), "2408:8256:f183:2d5c:79a1:3867:6acb:358b");
    ASSERT_TRUE(e1);

    auto data = e1.AsInAddr();
    e2.Assign(data);
    ASSERT_EQ(e2.AsString(), "2408:8256:f183:2d5c:79a1:3867:6acb:358b");
    ASSERT_TRUE(e2);
}

TEST(TOOLS, v4_EndPointInfo) {
    using namespace wutils::network;
    ip::v4::EndPointInfo e1;
    ip::v4::EndPointInfo e2;

    ASSERT_FALSE(e1);
    ASSERT_TRUE(e1.Assign(ip::v4::Address("127.0.0.1"), 4000));
    ASSERT_TRUE(e1);
    auto [ip, port] = e1.Dump();
    ASSERT_EQ(ip, "127.0.0.1");
    ASSERT_EQ(port, 4000);

    ASSERT_FALSE(e2);
    e2.Assign(*e1.AsSockAddr());
    ASSERT_TRUE(e2);
    auto [ip2, port2] = e2.Dump();
    ASSERT_EQ(ip2, "127.0.0.1");
    ASSERT_EQ(port2, 4000);
}

TEST(TOOLS, v6_EndPointInfo) {
    using namespace wutils::network;
    ip::v6::EndPointInfo e1;
    ip::v6::EndPointInfo e2;

    ASSERT_FALSE(e1);
    ASSERT_TRUE(e1.Assign(ip::v6::Address("2408:8256:f183:2d5c:79a1:3867:6acb:358b"), 4000));
    ASSERT_TRUE(e1);
    auto [ip, port] = e1.Dump();
    ASSERT_EQ(ip, "2408:8256:f183:2d5c:79a1:3867:6acb:358b");
    ASSERT_EQ(port, 4000);

    ASSERT_FALSE(e2);
    e2.Assign(*e1.AsSockAddr());
    ASSERT_TRUE(e2);
    auto [ip2, port2] = e2.Dump();
    ASSERT_EQ(ip2, "2408:8256:f183:2d5c:79a1:3867:6acb:358b");
    ASSERT_EQ(port2, 4000);
}

TEST(TOOLS, unix_EndPointInfo) {
    using namespace wutils::network;
    Unix::EndPointInfo e1;
    Unix::EndPointInfo e2;

    ASSERT_FALSE(e1);
    ASSERT_FALSE(e1.Assign("temp.sock"));
    ASSERT_FALSE(e1);
    ASSERT_TRUE(e1.Assign("/temp.sock"));
    ASSERT_TRUE(e1);
    auto [path, is] = e1.Dump();
    ASSERT_EQ(path, "/temp.sock");
    ASSERT_EQ(is, false);

    ASSERT_FALSE(e2);
    e2.Assign(*e1.AsSockAddr());
    ASSERT_TRUE(e2);
    auto [path2, is2] = e2.Dump();
    ASSERT_EQ(path2, "/temp.sock");
    ASSERT_EQ(is2, false);
}

TEST(SOCKET, tcp_socket) {
    using namespace wutils::network;
    ip::tcp::Socket<ip::v4> socket0(nullptr);
    ip::v4::EndPointInfo    e1(ip::v4::Address("127.0.0.1"), 4000);
    ip::v4::EndPointInfo    e2;

    ip::tcp::Acceptor<ip::v4> acceptor;
    ip::tcp::Socket<ip::v4>   cli;

    DEFER([&]() {
        socket0.Close();
        acceptor.Close();
        cli.Close();

        ASSERT_FALSE(socket0);
        ASSERT_FALSE(acceptor);
        ASSERT_FALSE(cli);
    });

    ASSERT_EQ(ip::tcp::SHUT_RD, ::SHUT_RD);
    ASSERT_EQ(ip::tcp::SHUT_WR, ::SHUT_WR);
    ASSERT_EQ(ip::tcp::SHUT_RDWR, ::SHUT_RDWR);

    ASSERT_TRUE(acceptor);
    ASSERT_TRUE(acceptor.Bind(e1));
    ASSERT_TRUE(acceptor.Listen());


    ASSERT_FALSE(socket0);
    ASSERT_TRUE(cli);
    ASSERT_TRUE(cli.Connect(e1));

    auto ser_cli = acceptor.Accept(e2);
    std::cout << "socket " << ser_cli.GetNativeSocket() << std::endl;

    DEFER([&]() {
        ser_cli.Close();
        ASSERT_FALSE(ser_cli);
    })

    ASSERT_TRUE(ser_cli);
    auto [ip, port] = e2.Dump();
    GTEST_LOG_(INFO) << "ip:" << ip << " port:" << port;

    char buf[1024];

    ASSERT_EQ(ser_cli.Send((uint8_t *)"123456", 6), 6);
    EXPECT_EQ(cli.Recv((uint8_t *)buf, 1024), 6);
    ASSERT_EQ(std::string("123456"), std::string(buf, 6));

    ASSERT_EQ(cli.Send((uint8_t *)"123456", 6), 6);
    EXPECT_EQ(ser_cli.Recv((uint8_t *)buf, 1024), 6);
    ASSERT_EQ(std::string("123456"), std::string(buf, 6));

    cli.ShutDown(ip::tcp::SHUT_WR);
    auto len = ser_cli.Recv((uint8_t *)buf, 1024);
    ASSERT_EQ(len, 0);
}

TEST(SOCKET, unix_socket) {
    using namespace wutils::network;
    Unix::Stream::Socket socket0(nullptr);
    std::string          path1 = "/tmp/wang/a.sock";
    std::string          path2 = "/tmp/wang/b.sock";
    std::string          path3 = "/tmp/wang/c.sock";
    Unix::EndPointInfo   e1;
    Unix::EndPointInfo   e2(path2);
    Unix::EndPointInfo   e22;
    Unix::EndPointInfo   e3(path3);
    Unix::EndPointInfo   e33;

    Unix::Stream::Acceptor acceptor;
    Unix::Stream::Socket   cli;
    Unix::Stream::Socket   cli2;

    DEFER([&]() {
        socket0.Close();
        acceptor.Close();
        cli.Close();
        cli2.Close();

        ASSERT_FALSE(socket0);
        ASSERT_FALSE(acceptor);
        ASSERT_FALSE(cli);
        ASSERT_FALSE(cli2);
    });

    ASSERT_FALSE(e1);
    ASSERT_TRUE(e1.Assign(path1));
    ASSERT_TRUE(e1);

    ASSERT_TRUE(acceptor) << acceptor.GetNativeSocket();
    ASSERT_TRUE(acceptor.Bind(e1)) << wutils::SystemError::GetSysErrCode();
    ASSERT_TRUE(acceptor.Listen());


    ASSERT_FALSE(socket0);
    ASSERT_TRUE(cli);
    ASSERT_TRUE(cli.Bind(e2)) << wutils::SystemError::GetSysErrCode();
    ASSERT_TRUE(cli.Connect(e1));

    auto ser_cli = acceptor.Accept(e22);
    std::cout << "socket " << ser_cli.GetNativeSocket() << std::endl;

    DEFER([&]() {
        ser_cli.Close();
        ASSERT_FALSE(ser_cli);
    })

    ASSERT_TRUE(ser_cli) << wutils::SystemError::GetSysErrCode();
    auto [ip, is] = e22.Dump();
    GTEST_LOG_(INFO) << "ip:" << ip << " create file:" << is;

    char buf[1024];

    ASSERT_EQ(ser_cli.Send((uint8_t *)"123456", 6), 6);
    EXPECT_EQ(cli.Recv((uint8_t *)buf, 1024), 6);
    ASSERT_EQ(std::string("123456"), std::string(buf, 6));

    ASSERT_EQ(cli.Send((uint8_t *)"123456", 6), 6);
    EXPECT_EQ(ser_cli.Recv((uint8_t *)buf, 1024), 6);
    ASSERT_EQ(std::string("123456"), std::string(buf, 6));

    ASSERT_TRUE(cli2);
    ASSERT_TRUE(cli2.Bind(e3)) << wutils::SystemError::GetSysErrCode();
    ASSERT_TRUE(cli2.Connect(e1));

    auto ser_cli2 = acceptor.Accept(e33);
    std::cout << "socket " << ser_cli2.GetNativeSocket() << std::endl;

    DEFER([&]() {
        ser_cli2.Close();
        ASSERT_FALSE(ser_cli2);
    })

    ASSERT_TRUE(ser_cli2) << wutils::SystemError::GetSysErrCode();
    auto [ip3, is3] = e33.Dump();
    GTEST_LOG_(INFO) << "ip:" << ip3 << " create file:" << is3;

    ASSERT_EQ(ser_cli2.Send((uint8_t *)"123456", 6), 6);
    EXPECT_EQ(cli2.Recv((uint8_t *)buf, 1024), 6);
    ASSERT_EQ(std::string("123456"), std::string(buf, 6));

    ASSERT_EQ(cli2.Send((uint8_t *)"123456", 6), 6);
    EXPECT_EQ(ser_cli2.Recv((uint8_t *)buf, 1024), 6);
    ASSERT_EQ(std::string("123456"), std::string(buf, 6));
}
