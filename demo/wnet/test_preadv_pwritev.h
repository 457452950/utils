#ifndef UTILS_DEMO_WNET_TEST_PREADV_H
#define UTILS_DEMO_WNET_TEST_PREADV_H

#include <iostream>

#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;


/**
 * test_preadv_pwritev
 */

namespace test_preadv_pwritev_config {

namespace srv {
namespace listen {
constexpr char     *ip     = "0:0:0:0:0:0:0:0";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace listen
} // namespace srv

namespace cli {
namespace connect {
constexpr char     *ip     = "0:0:0:0:0:0:0:0";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace connect
} // namespace cli

void server_thread() {
    using namespace wutils;

    auto sock = MakeSocket(srv::listen::family, srv::listen::protol);
    SetSocketReuseAddr(sock, false);
    // int  res  = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);

    EndPoint srv_ed;
    if(!srv_ed.Assign(srv::listen::ip, srv::listen::port, srv::listen::family)) {
        return;
    }
    int srv = Bind(sock, srv_ed);
    if(!srv) {
        cout << "bind error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "bind ok" << endl;
    }

    int lis_res = listen(sock, 5);
    if(lis_res == -1) {
        cout << "listen error : " << errno << endl;
        return;
    } else {
        cout << "listen ok" << endl;
    }

    EndPoint en;
    srv = Accept4(sock, en, SOCK_NONBLOCK);
    // ok  = SetSocketNonBlock(srv);
    // if(!ok) {
    //     cout << "[test_preadv_pwritev]SetSocketNonBlock error : " << strerror(errno) << endl;
    // } else {
    //     cout << "SetSocketNonBlock ok" << endl;
    // }

    if(srv == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok " << srv << endl;
    }

    int totol       = 0;
    auto [ip, port] = EndPoint::Dump(en);
    cout << "client : ip " << ip << " port:" << port << endl;

    const char   str0[]    = "helasdadafsafaafsadfdsaasfasfafafasfaufasnukfgagfasnjknfajkfbjasbfab,"
                             "gkhjbgherabllhjbfahjlbehjrlgbhaejrbgjaelrkbghaerjbghajergberhjgbeshjbl";
    const char  *str1      = "1\n";
    const char  *str2      = "2\n";
    const char  *str3      = "3\n";
    const char  *str4      = "4\n";
    const char  *str5      = "5\n";
    const char  *str6      = "6\n";
    const char  *str7      = "7\n";
    const char  *str8      = "8\n";
    const char  *str9      = "9\n";
    const char   strs[][1] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    // char        *str10 = "10\n";
    struct iovec iov[2];
    ssize_t      nwritten;

    iov[0].iov_base = (void *)str0;
    iov[0].iov_len  = strlen(str0);
    iov[1].iov_base = (void *)strs[0];
    iov[1].iov_len  = 1;

    // send(srv, str0, strlen(str0), 0);

    for(size_t i = 0; i < 100; i++) {
        iov[1].iov_base = (void *)strs[i % 10];

        // nwritten = pwritev2(srv, iov, 2, -1, RWF_APPEND);
        nwritten = pwritev2(srv, iov, 2, -1, 0);
        // nwritten        = writev(srv, iov, 2);
        // nwritten = send(srv, str0, strlen(str0), 0);

        totol += nwritten;
        if(nwritten == 140)
            continue;
        cout << "pwritev2 size " << nwritten << endl;
        if(nwritten == -1) {
            auto err = SystemError::GetSysErrCode();
            cout << iov[0].iov_len << " " << iov[1].iov_len << endl;
            cout << err << " " << EAGAIN << endl;
            cout << "pwritev2 error " << err << endl;
            break;
        }
    }
    cout << "send size totol : " << totol << endl;

    ::close(srv);
    ::close(sock);
}

void client_thread() {
    using namespace wutils;

    EndPoint srv_ed;
    if(!srv_ed.Assign(cli::connect::ip, cli::connect::port, cli::connect::family)) {
        return;
    }

    auto cli = MakeSocket(cli::connect::family, cli::connect::protol);
    bool ok  = ConnectToHost(cli, srv_ed);
    cout << "[test_preadv_pwritev]ConnectToHost error : " << strerror(errno) << endl;
    if(!ok) {
        cout << "[test_preadv_pwritev]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
        // SetSocketNonBlock(cli);
    }

    {
        struct iovec iov[2];
        iov[0].iov_base = new char[1024];
        iov[0].iov_len  = 1024;
        iov[1].iov_base = new char[1024];
        iov[1].iov_len  = 1024;
        auto len        = readv(cli, iov, 2);
        cout << " : " << iov[0].iov_len << std::endl << " : " << iov[1].iov_len << std::endl;
        delete[](char *)iov[0].iov_base;
        delete[](char *)iov[1].iov_base;
    }

    {
        char recv_buf[150000]{0};
        auto res = ::recv(cli, recv_buf, 150000, 0);
        // cout << "'" << std::string(recv_buf_, res).c_str() << "' \n recv_buf_ len : " << strlen(recv_buf_) << endl;
        cout << "recv res : " << res << endl;
        if(res == -1) {
            auto err = SystemError::GetSysErrCode();
            cout << err << " " << EWOULDBLOCK << endl;
            cout << "recv error " << err << endl;
        }
    }
    {
        char    recv_buf[150000]{0};
        ssize_t res = ::recv(cli, recv_buf, 150000, 0);
        std::cout.flush();
        // cout << "'" << std::string(recv_buf_, 100).c_str() << "' \n" << endl;
        // printf("' %c'", recv_buf_[0]);
        res = ::recv(cli, recv_buf, 150000, 0);
        cout << "recv res : " << res << endl;
        if(res == -1) {
            auto err = SystemError::GetSysErrCode();
            cout << err << " " << EWOULDBLOCK << endl;
            cout << "recv error " << err << endl;
        }
    }

    ::close(cli);
}

} // namespace test_preadv_pwritev_config


void test_preadv_pwritev() {
    //    using namespace test_preadv_pwritev_config;
    //
    //    thread sr(server_thread);
    //    thread cl(client_thread);
    //
    //    sr.join();
    //    cl.join();
}

#endif