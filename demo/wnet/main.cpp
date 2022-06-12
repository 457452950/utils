#include <csignal>
#include <iostream>
#include "TestSession.h"

using namespace std;
using namespace wlb::NetWork;

void handle_pipe(int singn) { cout << "singn" << endl; }

int main() {

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    try {
        //        WSingleTcpServer *server = new WSingleTcpServer();
        //        if (!server->Init()) {
        //            std::cout << "server init failed" << std::endl;
        //        }
        WMultiTcpServer *server = new WMultiTcpServer();
        if(!server->Init(1)) {
            std::cout << "server init failed" << std::endl;
        }
        server->AddAccepter("127.0.0.1", 4000);
        server->run();

        auto s = MakeTcpV4Socket();
        wlb::NetWork::ConnectToHost(s, "127.0.0.1", 4000);
        //        close(s);
        //        shutdown(s, SHUT_RDWR);
        shutdown(s, SHUT_WR);
        //        sleep(5);
        //        shutdown(s, SHUT_RD);
        sleep(5);
        auto l = ::send(s, "123", 3, 0);
        printf("send len : %ld", l);
        char aa[3];
        l = ::recv(s, aa, 3, 0);
        printf("recv len : %ld, %s", l, aa);

        server->WaitForQuit();
        server->Close();
        server->Destroy();

        delete server;
    } catch(const std::exception &e) {
        std::cerr << e.what() << '\n';
    }
}
