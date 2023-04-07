#ifndef UTILS_WRSA_H
#define UTILS_WRSA_H

#include <iostream>
#include <memory>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

using RSA_ptr  = std::unique_ptr<RSA, decltype(RSA_free) *>;
using RSA_wptr = std::weak_ptr<RSA>;

class WRSAPubKey {
public:
    explicit WRSAPubKey();
    explicit WRSAPubKey(RSA_ptr &key);
    ~WRSAPubKey() {}

private:
    RSA_ptr rsa_context_;

public:
    int8_t GetRSASize();
    //
    bool WriteFormatToFile(const std::string &path);
    bool ReadFromFormatFile(const std::string &filepath);
    //
    /**
     *  if error return -1, else return the encode message len
     */
    int16_t Encode(const uint8_t *input_str, int16_t str_len, uint8_t *output_str);
    int16_t Decode(const uint8_t *input_str, uint8_t *output_str);
};
WRSAPubKey::WRSAPubKey() : rsa_context_(nullptr, RSA_free) {}
bool WRSAPubKey::ReadFromFormatFile(const std::string &filepath) {
    BIO *pBIO = BIO_new_file(filepath.c_str(), "rb");
    if(!pBIO) {
        return false;
    }

    auto r = PEM_read_bio_RSA_PUBKEY(pBIO, nullptr, nullptr, nullptr);

    if(r != nullptr) {
        rsa_context_ = RSA_ptr(r, RSA_free);
        BIO_free(pBIO);
        return true;
    }
    std::cout << "PEM_read_bio_RSA_PUBKEY false" << std::endl;
    BIO_free(pBIO);
    return false;
}
WRSAPubKey::WRSAPubKey(RSA_ptr &key) : rsa_context_(std::move(key)) {}
int8_t WRSAPubKey::GetRSASize() { return RSA_size(this->rsa_context_.get()); }
bool   WRSAPubKey::WriteFormatToFile(const std::string &path) {
    BIO *pBIO = nullptr;
    int  iRV  = 0;

    /* PEM编码 无密码 私钥文件*/
    pBIO = BIO_new_file(path.c_str(), "w");
    if(!pBIO) {
        return false;
    }
    iRV = PEM_write_bio_RSA_PUBKEY(pBIO, this->rsa_context_.get());
    if(iRV != 1) {
        BIO_free(pBIO);
        return false;
    }
    BIO_free(pBIO);
    return true;
}
int16_t WRSAPubKey::Encode(const uint8_t *input_str, int16_t str_len, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());
    // std::cout << "rsa_len : " << rsa_len << std::endl;
    rsa_len -= 42;
    rsa_len = (rsa_len < str_len) ? rsa_len : str_len;
    // std::cout << "input_len : " << rsa_len << std::endl;
    int16_t res = RSA_public_encrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_OAEP_PADDING);
    return (res == -1) ? -1 : rsa_len;
}
int16_t WRSAPubKey::Decode(const uint8_t *input_str, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());
    // std::cout << "input_len : " << rsa_len << std::endl;
    int16_t res = RSA_public_decrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_PADDING);
    return res;
}


class WRSAPriKey {
public:
    explicit WRSAPriKey() : rsa_context_(nullptr, RSA_free) {}
    explicit WRSAPriKey(RSA_ptr &key);
    ~WRSAPriKey() {}

private:
    RSA_ptr rsa_context_;

public:
    int8_t GetRSASize();
    //
    bool WriteFormatToFile(const std::string &path);
    bool WriteFormatToFile(const std::string &path, const std::string &password);
    bool ReadFromFormatFile(const std::string &filepath);
    bool ReadFromFormatFile(const std::string &filepath, const std::string &password);
    //
    int16_t Encode(const uint8_t *input_str, int16_t str_len, uint8_t *output_str);
    /**
     * if error return -1, else return the output message len
     */
    int16_t Decode(const uint8_t *input_str, uint8_t *output_str);
};
WRSAPriKey::WRSAPriKey(RSA_ptr &key) : rsa_context_(std::move(key)) {}
int8_t WRSAPriKey::GetRSASize() { return RSA_size(this->rsa_context_.get()); }
bool   WRSAPriKey::WriteFormatToFile(const std::string &path) {
    BIO *pBIO = nullptr;
    int  iRV  = 0;

    /* PEM编码 无密码 私钥文件*/
    pBIO = BIO_new_file(path.c_str(), "w");
    if(!pBIO) {
        std::cout << "BIO_new_file false" << std::endl;
        return false;
    }
    iRV = PEM_write_bio_RSAPrivateKey(pBIO, this->rsa_context_.get(), NULL, NULL, 0, NULL, NULL);
    if(iRV != 1) {
        std::cout << "PEM_write_bio_RSAPrivateKey false" << std::endl;
        BIO_free(pBIO);
        return false;
    }
    BIO_free(pBIO);
    return true;
}
bool WRSAPriKey::ReadFromFormatFile(const std::string &filepath) {
    BIO *pBIO = BIO_new_file(filepath.c_str(), "rb");
    if(!pBIO) {
        std::cout << "BIO_new_file false" << std::endl;
        return false;
    }

    auto r = PEM_read_bio_RSAPrivateKey(pBIO, nullptr, nullptr, nullptr);
    if(r != nullptr) {
        rsa_context_ = RSA_ptr(r, RSA_free);
        BIO_free(pBIO);
        return true;
    }
    std::cout << "PEM_read_bio_RSAPrivateKey false" << std::endl;
    BIO_free(pBIO);
    return false;
}
bool WRSAPriKey::WriteFormatToFile(const std::string &path, const std::string &password) {
    BIO *     pBIO    = nullptr;
    EVP_PKEY *pEVPKey = NULL;
    int       iRV     = 0;

    pEVPKey = EVP_PKEY_new();
    //将RSA对象赋给EVP_PKEY对象
    EVP_PKEY_assign_RSA(pEVPKey, this->rsa_context_.get());
    RSA_up_ref(this->rsa_context_.get());

    /* PEM编码 私钥文件*/
    pBIO = BIO_new_file(path.c_str(), "w");
    if(!pBIO) {
        EVP_PKEY_free(pEVPKey);
        return false;
    }
    iRV = PEM_write_bio_PKCS8PrivateKey(pBIO, pEVPKey, EVP_des_ede3_cbc(), NULL, 0, 0, (void *)password.c_str());
    if(iRV != 1) {
        EVP_PKEY_free(pEVPKey);
        BIO_free(pBIO);
        return false;
    }

    EVP_PKEY_free(pEVPKey);
    BIO_free(pBIO);
    return true;
}

bool WRSAPriKey::ReadFromFormatFile(const std::string &filepath, const std::string &password) {
    BIO *pBIO = BIO_new_file(filepath.c_str(), "rb");
    if(!pBIO) {
        return false;
    }

    auto e = PEM_read_bio_PrivateKey(pBIO, nullptr, nullptr, (void *)password.c_str());

    if(e != nullptr) {
        auto r = EVP_PKEY_get1_RSA(e);
        RSA_up_ref(r);
        rsa_context_ = RSA_ptr(r, RSA_free);

        EVP_PKEY_free(e);
        BIO_free(pBIO);
        RSA_free(r);
        return true;
    }

    std::cout << "PEM_read_bio_PrivateKey false" << std::endl;
    BIO_free(pBIO);
    return false;
}

int16_t WRSAPriKey::Encode(const uint8_t *input_str, int16_t str_len, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());
    std::cout << "rsa_len : " << rsa_len << std::endl;

    rsa_len -= 11;
    rsa_len = (rsa_len < str_len) ? rsa_len : str_len;
    std::cout << "input_len : " << rsa_len << std::endl;

    int16_t res = RSA_private_encrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_PADDING);

    if(res == -1) {
        std::cout << ERR_reason_error_string(ERR_get_error()) << std::endl;
    }

    return (res == -1) ? -1 : rsa_len;
}
int16_t WRSAPriKey::Decode(const uint8_t *input_str, uint8_t *output_str) {
    auto rsa_len = RSA_size(this->rsa_context_.get());
    // std::cout << "input_len : " << rsa_len << std::endl;
    int16_t res = RSA_private_decrypt(rsa_len, input_str, output_str, this->rsa_context_.get(), RSA_PKCS1_OAEP_PADDING);
    return res;
}


class WRSA {
public:
    WRSA();
    ~WRSA();

    bool                        Init(const int bits = 1024, unsigned int e = RSA_3);
    std::shared_ptr<WRSAPubKey> GetPublicKey();
    std::shared_ptr<WRSAPriKey> GetPrivatecKey();

private:
    RSA_ptr rsa_context_;
};

WRSA::WRSA() : rsa_context_(RSA_new(), RSA_free) {}
WRSA::~WRSA() {}

bool WRSA::Init(const int bits, unsigned int e) {
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
    std::cout << "rsa_len : " << rsa_len * 8 << std::endl;
    BN_free(E);
    return true;
}

std::shared_ptr<WRSAPubKey> WRSA::GetPublicKey() {
    RSA_ptr pub_rsa(RSAPublicKey_dup(this->rsa_context_.get()), RSA_free);

    return std::make_shared<WRSAPubKey>(pub_rsa);
}

std::shared_ptr<WRSAPriKey> WRSA::GetPrivatecKey() {
    RSA_ptr pri_rsa(RSAPrivateKey_dup(this->rsa_context_.get()), RSA_free);

    return std::make_shared<WRSAPriKey>(pri_rsa);
}


#endif // UTILS_WRSA_H