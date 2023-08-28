#include <iostream>
#include <cassert>
#include <filesystem>
#include <future>

#include "wutils/logger/StreamLogger.h"
#include "wutils/network/easy/Tcp.h"
#include "wutils/network/ssl/ssl.h"
#include "wutils/base/Defer.h"

using namespace std;
using namespace wutils;
using namespace wutils::network;

#define SERVER_IP ("0.0.0.0")
#define SERVER_PORT (12000)
#define SERVER_PRO (AF_FAMILY::INET)

NetAddress server_na;

int sssl_verify_cb(int preverify_ok, X509_STORE_CTX *x509_ctx) {
    LOG(LINFO, "server") << "is verify ok ? " << preverify_ok << " "
                         << X509_verify_cert_error_string(X509_STORE_CTX_get_error(x509_ctx));

    switch(X509_STORE_CTX_get_error(x509_ctx)) {
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        // return 1 when pass
        preverify_ok = 1;
    default:
        break;
    }
    return preverify_ok;
}

void server_thread() {
    assert(std::filesystem::exists("/tmp/cacert.pem"));

    auto ctx = ssl::SslContext::Create(ssl::SslContext::TLS, ssl::SslContext::SERVER);

    //    ctx->SetVerify(ssl::SslContext::VerifyType::FAIL_IF_NO_PEER_CERT, sssl_verify_cb);
    ctx->SetCertificatePassword("wang");

    auto err = ctx->LoadCertificateFile("/tmp/cacert.pem", ssl::SslContext::PEM);
    if(err) {
        LOG(LERROR, "server") << "load file error : " << err.message();
        return;
    } else
        LOG(LINFO, "server") << "load file success : " << err.message();

    err = ctx->LoadPrivateKey("/tmp/privkey.pem", ssl::SslContext::PEM);
    if(err) {
        LOG(LERROR, "server") << "load file error : " << err.message();
        return;
    } else
        LOG(LINFO, "server") << "load file success : " << err.message();

    err = ctx->CheckPrivateKey();
    if(err) {
        LOG(LERROR, "server") << "check private key error : " << err.message();
        return;
    } else {
        LOG(LINFO, "server") << "check private key success : " << err.message();
    }

    // normal socket

    if(!server_na.Assign(SERVER_IP, SERVER_PORT, SERVER_PRO)) {
        LOG(LERROR, "server") << "server net address assign fail." << GetGenericError().message();
        return;
    }

    tcp::Socket server;
    if(!server.Open(server_na.GetFamily())) {
        LOG(LERROR, "server") << "server open fail." << GetGenericError().message();
        return;
    }
    DEFER([&]() { server.Close(); });
    if(!server.SetAddrReuse(true)) {
        LOG(LERROR, "server") << "server addr reuse fail." << GetGenericError().message();
        return;
    }
    if(!server.Bind(server_na)) {
        LOG(LERROR, "server") << "server bind fail." << GetGenericError().message();
        return;
    }
    if(!server.Listen()) {
        LOG(LERROR, "server") << "server listen fail." << GetGenericError().message();
        return;
    }

    // ssl

    auto ser_cli = server.Accept(nullptr);
    if(!ser_cli) {
        LOG(LERROR, "server") << "server accept fail. " << GetGenericError().message();
        return;
    }
    DEFER([&]() { ser_cli.Close(); });

    auto ssl_ser_cli = ctx->NewSocket(ser_cli);
    DEFER([&]() { ssl_ser_cli.Close(); });

    err = ssl_ser_cli.Open();
    if(err) {
        LOG(LERROR, "server") << "ssl socket open error : " << err.message();
        return;
    } else {
        LOG(LINFO, "server") << "ssl socket open success : " << err.message();
    }

    //    err = ssl_ser_cli.SslAccept();
    //    if(err) {
    //        LOG(LERROR, "server") << "ssl socket ssl accept error : " << err.message();
    //        return;
    //    } else {
    //        LOG(LINFO, "server") << "ssl socket ssl accept success : " << err.message();
    //    }
    err = ssl_ser_cli.SslDoHandShake();
    if(err) {
        LOG(LERROR, "server") << "ssl socket ssl accept error : " << err.message();
        return;
    } else {
        LOG(LINFO, "server") << "ssl socket ssl accept success : " << err.message();
    }

    auto res = ssl_ser_cli.SslVerifyResult();
    if(res) {
        LOG(LERROR, "server") << "verify result. error : " << X509_verify_cert_error_string(res);
        return;
    } else {
        LOG(LINFO, "server") << "ssl verify pass.  success : " << err.message();
    }

    char send_buf[] = "sadsadsadsafaasf";
    char recv_buf[1024];

    err = ssl_ser_cli.SslWrite(send_buf, std::size(send_buf));
    if(err) {
        LOG(LERROR, "server") << "ssl socket ssl send error : " << err.message();
        return;
    } else {
        LOG(LINFO, "server") << "ssl socket ssl send success : " << err.message();
    }

    std::tie(std::ignore, err) = ssl_ser_cli.SslRead(recv_buf, size(recv_buf));
    if(err) {
        LOG(LERROR, "server") << "ssl socket ssl read error : " << err.message();
        return;
    } else {
        LOG(LINFO, "server") << "ssl socket ssl read success : " << err.message() << " " << recv_buf;
    }
}

bool verify_ok{false};

int ssl_verify_cb(int preverify_ok, int error) {
    LOG(LINFO, "client") << "is verify ok ? " << preverify_ok << " " << X509_verify_cert_error_string(error);

    switch(error) {
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        // return 1 when pass
        preverify_ok = 1;
        break;
    default:
        break;
    }

    verify_ok = preverify_ok;
    return preverify_ok;
}

void client_thread() {
    auto ctx = ssl::SslContext::Create(ssl::SslContext::TLS, ssl::SslContext::CLIENT);

    ctx->SetVerify(ssl::SslContext::VerifyType::Peer, ssl_verify_cb);

    std::this_thread::sleep_for(50ms);

    tcp::Socket client;
    if(!client.Open(server_na.GetFamily())) {
        LOG(LERROR, "client") << "client open fail." << GetGenericError().message();
        return;
    }
    DEFER([&]() { client.Close(); });
    if(!client.Connect(server_na)) {
        LOG(LERROR, "client") << "client connect fail." << GetGenericError().message();
        return;
    }

    auto ssl_cli = ctx->NewSocket(client);
    DEFER([&]() { ssl_cli.Close(); });

    auto err = ssl_cli.Open();
    if(err) {
        LOG(LERROR, "client") << "ssl socket open error : " << err.message();
        return;
    } else {
        LOG(LINFO, "client") << "ssl socket open success : " << err.message();
    }

    //    err = ssl_cli.SslConnect();
    //    if(err) {
    //        LOG(LERROR, "client") << "ssl socket ssl connect error : " << err.message();
    //        return;
    //    } else {
    //        LOG(LINFO, "client") << "ssl socket ssl connect success : " << err.message();
    //    }
    err = ssl_cli.SslDoHandShake();
    if(err) {
        LOG(LERROR, "client") << "ssl socket ssl connect error : " << err.message();
        return;
    } else {
        LOG(LINFO, "client") << "ssl socket ssl connect success : " << err.message();
    }

    auto res = ssl_cli.SslVerifyResult();
    //    auto res = verify_ok;
    if(!res) {
        LOG(LERROR, "client") << "verify res. error : " << X509_verify_cert_error_string(res);
        return;
    } else {
        LOG(LINFO, "client") << "ssl verify pass.  success : " << err.message();
    }

    char recv_buf[1024];

    std::tie(std::ignore, err) = ssl_cli.SslRead(recv_buf, size(recv_buf));
    if(err) {
        LOG(LERROR, "client") << "ssl socket ssl read error : " << err.message();
        return;
    } else {
        LOG(LINFO, "client") << "ssl socket ssl read success : " << err.value() << " " << err.message() << " "
                             << recv_buf;
    }

    err = ssl_cli.SslWrite(recv_buf, std::strlen(recv_buf));
    if(err) {
        LOG(LERROR, "client") << "ssl socket ssl send error : " << err.message();
        return;
    } else {
        LOG(LINFO, "client") << "ssl socket ssl send success : " << err.message();
    }
}

int main(int argc, char **argv) {
    wutils::log::Logger::GetInstance()->LogCout()->LogFile("/tmp/ssl.log")->SetLogLevel(LDEBUG)->Start();

    ssl::InitSsl();

    LOG(LINFO, "main") << "version " << ssl::SSLVersion() << " m version" << ssl::SSLMajorVersion();

    std::thread srv(server_thread);
    std::thread cli(client_thread);

    srv.join();
    cli.join();

    wutils::log::Logger::GetInstance()->StopAndWait();
    return 0;
}
