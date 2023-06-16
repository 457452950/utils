#ifndef UTILS_DEMO_WNET_TEST_IOVEC_H
#define UTILS_DEMO_WNET_TEST_IOVEC_H

#include <iostream>

#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;


/**
 * test_iovec
 */

namespace test_iovec_config {

namespace srv {
namespace listen {
constexpr char     *ip     = "127.0.0.1";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace listen
} // namespace srv

namespace cli {
namespace connect {
constexpr char     *ip     = "127.0.0.1";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace connect
} // namespace cli

void server_thread() {

    using namespace wutils;
    // listen

    EndPointInfo srv_ed;
    if(!srv_ed.Assign(srv::listen::ip, srv::listen::port, srv::listen::family)) {
        return;
    }

    auto sock = MakeListenedSocket(srv_ed, true);
    if(sock == -1) {
        return;
    }

    EndPointInfo en;
    // auto          srv = Accept4(sock, en, SOCK_NONBLOCK);
    auto srv = Accept4(sock, en, 0);
    if(srv == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok " << srv << endl;
    }

    auto info = EndPointInfo::Dump(en);
    cout << "client : ip " << std::get<0>(info) << " port:" << std::get<1>(info) << endl;

    for(int i = 0; i < 3; ++i) {
        const char *str0 = "hello this is message 123";
        ssize_t     nwritten;

        // send(srv, str0, strlen(str0), 0);

        for(size_t j = 0; j < 3; j++) {

            nwritten = send(srv, str0, strlen(str0), 0);

            cout << "send size " << nwritten << endl;
            if(nwritten == -1) {
                auto err = SystemError::GetSysErrCode();
                cout << "pwritev2 error " << err << endl;
                break;
            }
        }

        char recv_buf[1500]{0};
        auto res = ::recv(srv, recv_buf, 1500, 0);
        cout << "recv res : " << res << endl;
        if(res == -1) {
            auto err = SystemError::GetSysErrCode();
            cout << "recv error " << err << endl;
        } else {
            recv_buf[res] = '\0';
            cout << "recv res : " << recv_buf << endl;
        }
    }

    ::close(srv);
    ::close(sock);
}

void client_thread() {
    using namespace wutils;

    EndPointInfo srv_ed;
    if(!srv_ed.Assign(cli::connect::ip, cli::connect::port, cli::connect::family)) {
        return;
    }

    auto cli = MakeSocket(cli::connect::family, cli::connect::protol);
    bool ok  = ConnectToHost(cli, srv_ed);
    if(!ok) {
        cout << "[test_preadv_pwritev]connect error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "connect ok" << endl;
        // SetSocketNoBlock(cli);
    }

    for(int i = 0; i < 3; ++i) {

        IOVec   vec1(5, 20);
        IOVec   vec2(5, 20);
        char    recv_buf[1500];
        ssize_t res;

        for(size_t j = 0; j < 1; j++) {
            vec1.Write([&](iovec *v, int l) -> int64_t {
                cout << "can " << l << endl;
                auto res = readv(cli, v, l);
                cout << "readv res : " << res << endl;
                if(res == -1) {
                    auto err = SystemError::GetSysErrCode();
                    cout << "readv error " << err << endl;
                }
                return res;
            });
        }
        // res = ::recv(cli, recv_buf_, 1500, 0);
        // cout << "recv res : " << res << endl;
        // if(res == -1) {
        //     auto err = GetError();
        //     cout << "recv error " << ErrorToString(err) << endl;
        // } else {
        //     recv_buf_[res] = '\0';
        //     cout << "recv res : " << recv_buf_ << endl;
        // }

        cout << "err " << SystemError::GetSysErrCode() << endl;
        // res = ::recv(cli, recv_buf_, 150000, 0);
        vec1.Read([&](const iovec *v, int l) -> int64_t {
            cout << "vec1.Read l:" << l << " " << v << endl;
            auto res = writev(cli, v, l);
            cout << "writev res : " << res << endl;
            if(res == -1) {
                auto err = SystemError::GetSysErrCode();
                cout << cli << " writev error " << err << endl;
            }
            return res;
        });
        // vec1.Read([&](const uint8_t* data, uint32_t data_len) -> int64_t {
        //     auto res = ::send(cli, data, data_len, 0);
        //     if(res == -1) {
        //         auto err = GetError();
        //         cout << cli << " send error " << ErrorToString(err) << endl;
        //     }
        //     return res;
        // });
    }

    ::close(cli);
}

} // namespace test_iovec_config


void test_iovec() {
    using namespace test_iovec_config;

    thread sr(server_thread);
    thread cl(client_thread);

    sr.join();
    cl.join();
}


#endif
