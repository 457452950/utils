#pragma once
#ifndef UTIL_NETWORK_SSL_H
#define UTIL_NETWORK_SSL_H

#include <vector>
#include <string>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "Error.h"
#include "wutils/SharedPtr.h"
#include "wutils/base/HeadOnly.h"

namespace wutils::network::ssl {


HEAD_ONLY unsigned long SSLVersion() { return OpenSSL_version_num(); }

HEAD_ONLY void InitSsl() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    SSL_load_error_strings();
    ERR_load_crypto_strings();
}
HEAD_ONLY void ReleaseSsl() { ERR_free_strings(); }


HEAD_ONLY std::vector<const SSL_METHOD *> Context_Init_Methos = {
        SSLv23_server_method(), SSLv23_client_method(), DTLS_server_method(), DTLS_client_method()};
class SslContext {
public:
    enum protocol : int8_t { TLS, DTLS };
    enum type : int8_t { SERVER, CLIENT };

    static shared_ptr<SslContext> Create(protocol p, type t) {
        auto context       = shared_ptr<SslContext>(new SslContext);
        context->protocol_ = p;
        context->type_     = t;
        context->context_  = SSL_CTX_new(Context_Init_Methos[2 * p + t]);
        return context;
    }

    ~SslContext() {
        SSL_CTX_free(this->context_);
        this->context_ = nullptr;
    }

    protocol Protocol() const noexcept { return protocol_; }
    type     Type() const noexcept { return type_; }

    enum file_type { DER = SSL_FILETYPE_ASN1, PEM = SSL_FILETYPE_PEM };
    Error LoadCertificateFile(const char *file_name, file_type type) {
        auto ok = SSL_CTX_use_certificate_file(this->context_, file_name, static_cast<int>(type));
        if(!ok) {
            return make_ssl_error_code(ERR_get_error());
        }
        return {};
    }


private:
    SslContext() = default;

private:
    ssl_ctx_st *context_{nullptr};
    protocol    protocol_{TLS};
    type        type_{SERVER};
};


} // namespace wutils::network::ssl

#endif // UTIL_NETWORK_SSL_H
