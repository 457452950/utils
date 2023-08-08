#include <iostream>
#include <cassert>
#include <filesystem>

#include "wutils/logger/StreamLogger.h"
#include "wutils/network/ssl/ssl.h"

using namespace std;
using namespace wutils;
using namespace wutils::network;

int main(int argc, char **argv) {
    try {
        wutils::log::Logger::GetInstance()->LogFile("/tmp/ssl.log")->SetLogLevel(LDEBUG)->Start();

        ssl::InitSsl();

        assert(std::filesystem::exists("/tmp/server.pem"));

        auto ctx = ssl::SslContext::Create(ssl::SslContext::TLS, ssl::SslContext::SERVER);
        auto err = ctx->LoadCertificateFile("/tmp/test_error.pem", ssl::SslContext::PEM);
        if(err) {
            LOG(LERROR, "main") << "load file error : " << err.message();
        } else
            LOG(LINFO, "main") << "load file success : " << err.message();

        ssl::ReleaseSsl();

        wutils::log::Logger::GetInstance()->StopAndWait();
    } catch(std::runtime_error err) {
        std::cout << "runtime error " << err.what() << std::endl;
    }
    return 0;
}
