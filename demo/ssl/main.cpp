#include <iostream>
#include <cassert>
#include <filesystem>

#include "wutils/network/ssl/ssl.h"

using namespace std;
using namespace wutils;
using namespace wutils::network;

int main(int argc, char **argv) {
    try {
        ssl::InitSsl();

        assert(std::filesystem::exists("/tmp/server.pem"));

        auto ctx = ssl::SslContext::Create(ssl::SslContext::TLS, ssl::SslContext::SERVER);
        auto err = ctx->LoadCertificateFile("/tmp/server.pem", ssl::SslContext::PEM);
        if(err) {
            std::cout << "load file error : " << err.message() << std::endl;
        }
        std::cout << "load file success : " << err.message() << std::endl;

        ssl::ReleaseSsl();
    } catch(std::runtime_error err) {
        std::cout << "runtime error " << err.what() << std::endl;
    }
    return 0;
}
