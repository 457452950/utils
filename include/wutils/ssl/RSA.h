#ifndef UTILS_WRSA_H
#define UTILS_WRSA_H

#include <iostream>


#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>


#include "wutils/SharedPtr.h"

namespace wutils {

using RSA_ptr  = unique_ptr<RSA, decltype(RSA_free) *>;
using RSA_wptr = weak_ptr<RSA>;

using BIO_ptr      = unique_ptr<BIO, decltype(BIO_free) *>;
using EVP_PKEY_ptr = unique_ptr<EVP_PKEY, decltype(EVP_PKEY_free) *>;

class RSAPubKey {
public:
    explicit RSAPubKey();
    explicit RSAPubKey(RSA_ptr &key);
    ~RSAPubKey() = default;

private:
    RSA_ptr rsa_context_;

public:
    int GetRSASize();
    //
    bool WriteFormatToFile(const std::string &path);
    bool ReadFromFormatFile(const std::string &filepath);
    //
    /**
     *  if error return -1, else return the encode message len
     */
    int Encode(const uint8_t *input_str, int str_len, uint8_t *output_str);
    int Decode(const uint8_t *input_str, uint8_t *output_str);
};

RSAPubKey::RSAPubKey() : rsa_context_(nullptr, RSA_free) {}

bool RSAPubKey::ReadFromFormatFile(const std::string &filepath) {
    BIO_ptr pBIO = unique_ptr<BIO, decltype(BIO_free) *>(BIO_new_file(filepath.c_str(), "rb"), BIO_free);

    if(!pBIO) {
        return false;
    }

    auto r = PEM_read_bio_RSA_PUBKEY(pBIO.get(), nullptr, nullptr, nullptr);

    if(r != nullptr) {
        rsa_context_ = RSA_ptr(r, RSA_free);
        return true;
    }

    return false;
}

RSAPubKey::RSAPubKey(RSA_ptr &key) : rsa_context_(std::move(key)) {}

int RSAPubKey::GetRSASize() { return RSA_size(this->rsa_context_.get()); }

bool RSAPubKey::WriteFormatToFile(const std::string &path) {
    BIO_ptr pBIO = unique_ptr<BIO, decltype(BIO_free) *>(nullptr, BIO_free);
    int     iRV  = 0;

    /* PEM编码 无密码 私钥文件*/
    pBIO.reset(BIO_new_file(path.c_str(), "w"));
    if(!pBIO) {
        return false;
    }
    iRV = PEM_write_bio_RSA_PUBKEY(pBIO.get(), this->rsa_context_.get());
    if(iRV != 1) {
        return false;
    }

    return true;
}
int RSAPubKey::Encode(const uint8_t *input_str, int str_len, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());

    rsa_len -= 42;
    rsa_len = (rsa_len < str_len) ? rsa_len : str_len;

    auto res = RSA_public_encrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_OAEP_PADDING);
    return (res == -1) ? -1 : rsa_len;
}
int RSAPubKey::Decode(const uint8_t *input_str, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());

    auto res = RSA_public_decrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_PADDING);
    return res;
}


class RSAPriKey {
public:
    explicit RSAPriKey() : rsa_context_(nullptr, RSA_free) {}
    explicit RSAPriKey(RSA_ptr &key);
    ~RSAPriKey() = default;

private:
    RSA_ptr rsa_context_;

public:
    int GetRSASize();
    //
    bool WriteFormatToFile(const std::string &path);
    bool WriteFormatToFile(const std::string &path, const std::string &password);
    bool ReadFromFormatFile(const std::string &filepath);
    bool ReadFromFormatFile(const std::string &filepath, const std::string &password);
    //
    int Encode(const uint8_t *input_str, int str_len, uint8_t *output_str);
    /**
     * if error return -1, else return the output message len
     */
    int Decode(const uint8_t *input_str, uint8_t *output_str);
};

RSAPriKey::RSAPriKey(RSA_ptr &key) : rsa_context_(std::move(key)) {}
int  RSAPriKey::GetRSASize() { return RSA_size(this->rsa_context_.get()); }
bool RSAPriKey::WriteFormatToFile(const std::string &path) {
    BIO_ptr pBIO = unique_ptr<BIO, decltype(BIO_free) *>(nullptr, BIO_free);
    int     iRV  = 0;

    /* PEM编码 无密码 私钥文件*/
    pBIO.reset(BIO_new_file(path.c_str(), "w"));
    if(!pBIO) {
        std::cout << "BIO_new_file false" << std::endl;
        return false;
    }
    iRV = PEM_write_bio_RSAPrivateKey(pBIO.get(), this->rsa_context_.get(), NULL, NULL, 0, NULL, NULL);
    if(iRV != 1) {
        std::cout << "PEM_write_bio_RSAPrivateKey false" << std::endl;
        return false;
    }
    return true;
}
bool RSAPriKey::ReadFromFormatFile(const std::string &filepath) {
    BIO_ptr pBIO = unique_ptr<BIO, decltype(BIO_free) *>(BIO_new_file(filepath.c_str(), "rb"), BIO_free);
    if(!pBIO) {
        return false;
    }

    auto r = PEM_read_bio_RSAPrivateKey(pBIO.get(), nullptr, nullptr, nullptr);
    if(r != nullptr) {
        rsa_context_ = RSA_ptr(r, RSA_free);
        return true;
    }

    return false;
}
bool RSAPriKey::WriteFormatToFile(const std::string &path, const std::string &password) {
    BIO_ptr      pBIO    = unique_ptr<BIO, decltype(BIO_free) *>(nullptr, BIO_free);
    EVP_PKEY_ptr pEVPKey = unique_ptr<EVP_PKEY, decltype(EVP_PKEY_free) *>(nullptr, EVP_PKEY_free);
    int          iRV     = 0;

    pEVPKey.reset(EVP_PKEY_new());
    if(!pEVPKey) {
        return false;
    }

    // 将RSA对象赋给EVP_PKEY对象
    EVP_PKEY_assign_RSA(pEVPKey.get(), this->rsa_context_.get());
    RSA_up_ref(this->rsa_context_.get());

    /* PEM编码 私钥文件*/
    pBIO.reset(BIO_new_file(path.c_str(), "w"));
    if(!pBIO) {
        return false;
    }

    iRV = PEM_write_bio_PKCS8PrivateKey(
            pBIO.get(), pEVPKey.get(), EVP_des_ede3_cbc(), nullptr, 0, nullptr, (void *)password.c_str());

    if(iRV != 1) {
        return false;
    }

    return true;
}

bool RSAPriKey::ReadFromFormatFile(const std::string &filepath, const std::string &password) {
    BIO_ptr pBIO = unique_ptr<BIO, decltype(BIO_free) *>(BIO_new_file(filepath.c_str(), "rb"), BIO_free);
    if(!pBIO) {
        return false;
    }

    auto e = PEM_read_bio_PrivateKey(pBIO.get(), nullptr, nullptr, (void *)password.c_str());

    if(e != nullptr) {
        auto r = EVP_PKEY_get1_RSA(e);
        RSA_up_ref(r);
        rsa_context_ = RSA_ptr(r, RSA_free);

        EVP_PKEY_free(e);
        RSA_free(r);
        return true;
    }

    return false;
}

int RSAPriKey::Encode(const uint8_t *input_str, int str_len, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());

    rsa_len -= 11;
    rsa_len = (rsa_len < str_len) ? rsa_len : str_len;

    int res = RSA_private_encrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_PADDING);

    if(res == -1) {
        std::cout << ERR_reason_error_string(ERR_get_error()) << std::endl;
    }

    return (res == -1) ? -1 : rsa_len;
}
int RSAPriKey::Decode(const uint8_t *input_str, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());
    int  res = RSA_private_decrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_OAEP_PADDING);
    return res;
}


class RSA {
public:
    RSA();
    ~RSA();

    bool                  Init(const int bits = 1024, unsigned int e = RSA_3);
    shared_ptr<RSAPubKey> GetPublicKey();
    shared_ptr<RSAPriKey> GetPrivatecKey();

private:
    RSA_ptr rsa_context_;
};

RSA::RSA() : rsa_context_(RSA_new(), RSA_free) {}
RSA::~RSA() {}

bool RSA::Init(const int bits, unsigned int e) {
    BIGNUM *E = BN_new();

    int res = BN_set_word(E, RSA_3);
    if(res != 1) {
        BN_free(E);
        return false;
    }

    res = RSA_generate_key_ex(this->rsa_context_.get(), bits, E, nullptr);
    if(res != 1) {
        BN_free(E);
        return false;
    }

    auto rsa_len = RSA_size(this->rsa_context_.get());
    BN_free(E);
    return true;
}

shared_ptr<RSAPubKey> RSA::GetPublicKey() {
    RSA_ptr pub_rsa(RSAPublicKey_dup(this->rsa_context_.get()), RSA_free);

    return make_shared<RSAPubKey>(pub_rsa);
}

shared_ptr<RSAPriKey> RSA::GetPrivatecKey() {
    RSA_ptr pri_rsa(RSAPrivateKey_dup(this->rsa_context_.get()), RSA_free);

    return make_shared<RSAPriKey>(pri_rsa);
}

} // namespace wutils

#endif // UTILS_WRSA_H