#pragma once
#ifndef UTIL_NETWORK_SSL_H
#define UTIL_NETWORK_SSL_H

#include <utility>
#include <vector>
#include <string>
#include <cassert>
#include <functional>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "Error.h"
#include "wutils/SharedPtr.h"
#include "wutils/base/HeadOnly.h"
#include "wutils/network/base/Native.h"
#include "wutils/network/base/ISocket.h"

namespace wutils::network::ssl {


HEAD_ONLY constexpr char *SSLVersion() { return OPENSSL_VERSION_STR; }
HEAD_ONLY int             SSLMajorVersion() {
    static_assert(OPENSSL_VERSION_MAJOR == 3);
    return OPENSSL_version_major();
}

// auto init, auto release
class SslEnvironment {
public:
    static void Init() { static SslEnvironment env; }
    ~SslEnvironment() { ReleaseSsl(); }

private:
    SslEnvironment() { InitSsl(); }

    static void InitSsl() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();

        SSL_load_error_strings();
        ERR_load_crypto_strings();
    }
    static void ReleaseSsl() { ERR_free_strings(); }
};

HEAD_ONLY void InitSsl() { SslEnvironment::Init(); }


class Socket {
public:
    Socket(ssl_st *s, ISocket &sock) : ssl_(s), socket_(sock) {}
    ~Socket() = default;

    Error Open() {
        ERR_clear_error();

        auto ok = SSL_set_fd(this->ssl_, this->socket_.Get());
        if(!ok) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    Error SslAccept() {
        ERR_clear_error();

        auto res = SSL_accept(this->ssl_);
        if(res == 1) {
            return {};
        }
        return make_ssl_error_code(ERR_get_error());
    }

    /**
     * @note 无法通过返回值判断 handshake 是否通过, 需要额外进行一次 SslRead() 的调用失败判断.
     * issues: https://github.com/openssl/openssl/issues/11118
     */
    Error SslConnect() {
        ERR_clear_error();

        auto ok = SSL_connect(this->ssl_);
        if(ok != 1) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    void  SetServerState() { SSL_set_accept_state(this->ssl_); }
    void  SetClientState() { SSL_set_connect_state(this->ssl_); }
    bool  IsServerState() { return SSL_is_server(this->ssl_); }
    Error SslDoHandShake() {
        auto res = SSL_do_handshake(this->ssl_);
        if(res == 1) {
            return {};
        }
        return make_ssl_error_code(ERR_get_error());
    }

    OSSL_HANDSHAKE_STATE SslGetState() { return SSL_get_state(this->ssl_); }

    long SslVerifyResult() { return SSL_get_verify_result(this->ssl_); }

    Error SslWrite(const void *buf, int num) {
        ERR_clear_error();

        assert(num);
        auto len = SSL_write(this->ssl_, buf, num);
        if(len <= 0) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    /**
     *
     * @param buf
     * @param num
     * @return 旧文档指示 0 和 -1 之间存在差异，并且 -1 是可重试的。相反，您应该调用 SSL_get_error（）
     * 来确定它是否可以重试。
     */
    std::tuple<int, Error> SslRead(void *buf, int num) {
        ERR_clear_error();

        assert(num);
        auto len = SSL_read(this->ssl_, buf, num);
        if(len <= 0) {
            return {len, make_ssl_error_code(ERR_get_error())};
        }
        return {len, {}};
    }

    Error SslShutDown() {
        ERR_clear_error();

        auto ok = SSL_shutdown(this->ssl_);
        if(ok != 1) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    void Close() {
        if(this->ssl_) {
            SSL_free(this->ssl_);
            this->ssl_ = nullptr;
        }
    }

private:
    ssl_st  *ssl_{nullptr};
    ISocket &socket_;
};

HEAD_ONLY std::vector<const SSL_METHOD *> Context_Init_Methos = {
        SSLv23_server_method(), SSLv23_client_method(), DTLS_server_method(), DTLS_client_method()};

class SslContext {
public:
    enum protocol : int8_t { TLS, DTLS };
    enum type : int8_t { SERVER, CLIENT };

    static shared_ptr<SslContext> Create(protocol p, type t) {
        SslEnvironment::Init();

        auto context       = shared_ptr<SslContext>(new SslContext);
        context->protocol_ = p;
        context->type_     = t;
        context->context_  = SSL_CTX_new(Context_Init_Methos[2 * p + t]);
        return context;
    }

    ~SslContext() {
        if(this->extra_data_) {
            SSL_CTX_set_app_data(this->context_, nullptr);
            delete this->extra_data_;
            this->extra_data_ = nullptr;
        }

        SSL_CTX_free(this->context_);
        this->context_ = nullptr;
    }

    protocol Protocol() const noexcept { return protocol_; }
    type     Type() const noexcept { return type_; }

    void SetCertificatePassword(const std::string &password) {
        this->password_ = password;

        if(this->password_.empty()) {
            SSL_CTX_set_default_passwd_cb(this->context_, nullptr);
            return;
        }
        SSL_CTX_set_default_passwd_cb(this->context_, &SslContext::pem_password_cb);
        SSL_CTX_set_default_passwd_cb_userdata(this->context_, (void *)password_.c_str());
    }

    enum file_type { DER = SSL_FILETYPE_ASN1, PEM = SSL_FILETYPE_PEM };
    Error LoadCertificateFile(const char *file_name, file_type type) {
        ERR_clear_error();

        auto ok = SSL_CTX_use_certificate_file(this->context_, file_name, static_cast<int>(type));
        if(!ok) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    Error LoadCertificateChainFile(const char *file_name) {
        ERR_clear_error();

        auto ok = SSL_CTX_use_certificate_chain_file(this->context_, file_name);
        if(!ok) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    Error LoadPrivateKey(const char *file_name, file_type type) {
        ERR_clear_error();

        auto ok = SSL_CTX_use_PrivateKey_file(this->context_, file_name, static_cast<int>(type));
        if(!ok) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    Error CheckPrivateKey() {
        ERR_clear_error();

        auto ok = SSL_CTX_check_private_key(this->context_);
        if(!ok) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }

    using VerifyCB = std::function<bool(bool, int)>;
    enum class VerifyType {
        /**
         * @服务器模式 服务器不会向客户端发送客户端证书请求，因此客户端不会发送证书。
         * @客户端模式 如果不使用匿名密码（默认禁用），服务器将发送一个将被检查的证书。证书验证过程的结果可以在 TLS/SSL
         * 握手后使用 SSL_get_verify_result（3） 函数进行检查。无论验证结果如何，握手都会继续。
         */
        None = SSL_VERIFY_NONE,

        /**
         * @服务器模式 服务器向客户端发送客户端证书请求。检查返回的证书（如果有）。如果验证过程失败，TLS/SSL
         * 握手将立即终止，并显示一条包含验证失败原因的警报消息。行为可以通过附加的SSL_VERIFY_FAIL_IF_NO_PEER_CERT、SSL_VERIFY_CLIENT_ONCE和SSL_VERIFY_POST_HANDSHAKE标志进行控制。
         * @客户端模式 验证服务器证书。如果验证过程失败，TLS/SSL
         * 握手将立即终止，并显示一条包含验证失败原因的警报消息。如果未发送服务器证书，则由于使用了匿名密码，则忽略SSL_VERIFY_PEER。
         */
        Peer = SSL_VERIFY_PEER,

        /**
         * @服务器模式 如果客户端未返回证书，则 TLS/SSL
         * 握手将立即终止，并显示“握手失败”警报。此标志必须与SSL_VERIFY_PEER一起使用。
         * @客户端模式 忽略
         */
        FAIL_IF_NO_PEER_CERT = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,

        /**
         * @服务器模式
         * 在连接过程中仅请求一次客户端证书。如果在初始握手期间请求了证书，则不要在重新协商或身份验证后再次请求客户端证书。此标志必须与SSL_VERIFY_PEER一起使用。
         * @客户端模式 忽略
         */
        CLIENT_ONCE = SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
    };
    void SetVerify(VerifyType type, VerifyCB cb) {
        if(extra_data_ == nullptr) {
            this->extra_data_       = new AppData;
            extra_data_->verify_cb_ = std::move(cb);
            SSL_CTX_set_app_data(this->context_, extra_data_);
        } else {
            extra_data_->verify_cb_ = cb;
        }
        SSL_CTX_set_verify(this->context_, static_cast<int>(type), &SslContext::verify_call_back_);
    }

    Socket NewSocket(ISocket &sock) const {
        auto s = SSL_new(this->context_);
        if(type_ == SERVER) {
            SSL_set_accept_state(s);
        } else {
            SSL_set_connect_state(s);
        }
        return Socket(s, sock);
    }

private:
    SslContext() = default;

    static int pem_password_cb(char *buf, int size, int rwflag, void *userdata) {
        strncpy(buf, (char *)userdata, size);
        return std::strlen(buf);
    };
    std::string password_;

    static int verify_call_back_(int preverified, X509_STORE_CTX *ctx) {
        if(ctx) {
            // get ssl form x509_store_ctx, with use SSL_get_ex_data_X509_STORE_CTX_idx;
            // doc: https://www.openssl.org/docs/man1.0.2/man3/SSL_get_ex_data_X509_STORE_CTX_idx.html
            if(SSL *ssl =
                       static_cast<SSL *>(::X509_STORE_CTX_get_ex_data(ctx, ::SSL_get_ex_data_X509_STORE_CTX_idx()))) {

                if(SSL_CTX *context = ::SSL_get_SSL_CTX(ssl)) {

                    auto ptr = static_cast<AppData *>(SSL_CTX_get_app_data(context));
                    assert(ptr);
                    if(ptr->verify_cb_) {
                        return ptr->verify_cb_(preverified != 0, X509_STORE_CTX_get_error(ctx)) ? 1 : 0;
                    }
                    return preverified;
                }
            }
        }
        return 0;
    };
    struct AppData {
        VerifyCB verify_cb_;
    } *extra_data_{nullptr};

private:
    ssl_ctx_st *context_{nullptr};
    protocol    protocol_{TLS};
    type        type_{SERVER};
};


} // namespace wutils::network::ssl

#endif // UTIL_NETWORK_SSL_H
