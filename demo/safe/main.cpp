#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

using namespace std;

#include "ssl/WRSA.h"

auto Now() {
    return chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

void testRun();

int main(int argc, char **argv) {
    WRSA rsa;
    rsa.Init(2048);

    auto pubk = rsa.GetPublicKey();
    auto prik = rsa.GetPrivatecKey();

    pubk->WriteFormatToFile("pubkey.pem");
    prik->WriteFormatToFile("prikey1.pem");
    prik->WriteFormatToFile("prikey2.pem", "123");

    WRSAPubKey pubk2;
    WRSAPriKey prik2;
    WRSAPriKey prik3;
    assert(pubk2.ReadFromFormatFile("pubkey.pem"));
    cout << "pubk2.ReadFromFormatFile ok" << endl;
    assert(prik2.ReadFromFormatFile("prikey1.pem"));
    cout << "prik2.ReadFromFormatFile ok" << endl;
    assert(prik3.ReadFromFormatFile("prikey2.pem", "123"));
    cout << "prik3.ReadFromFormatFile ok" << endl;
    
    char text[] = "41864841564s23fsef65se41f65se1f56se1f51se1f5s1e56f1e6s51seg151s6g1s65g18656116515615611sd56f1e6r51eg561er6g1g65er1g56er1g65er1g";
    char pub_en[1024];
    char pri_de[1024];

    int res = pubk2.Encode((uint8_t*)text, strlen(text), (uint8_t*)pub_en);
    cout << "res : " << res << endl;

    res = prik2.Decode((uint8_t*)pub_en, (uint8_t*)pri_de);
    cout << "res : " << res << endl;

    cout << "after decode : " << pri_de << endl;
    

    res = prik2.Encode((uint8_t*)text, strlen(text), (uint8_t*)pub_en);
    cout << "res : " << res << endl;

    res = pubk2.Decode((uint8_t*)pub_en, (uint8_t*)pri_de);
    cout << "res : " << res << endl;

    cout << "after decode : " << pri_de << endl;

    auto now = Now();
    for (int i = 0; i < 100; ++i) {
        int res = pubk2.Encode((uint8_t*)text, strlen(text), (uint8_t*)pub_en);
        // cout << "res : " << res << endl;

        res = prik2.Decode((uint8_t*)pub_en, (uint8_t*)pri_de);
        // cout << "res : " << res << endl;
    }
    now = Now() - now;
    cout << "pass : " <<  now << endl;
    cout << "pass : " << 100 * strlen(text) / now * 1000 << endl;

    return 0;
}



void testRun() {
    // basic
    RSA *   rsa     = RSA_new();
    BIGNUM *rsa_3   = BN_new();
    RSA *   pub_rsa = nullptr;
    RSA *   pri_rsa = nullptr;
    char    cout_text[5024];

    // remote endpoint
    BIO *bio_mem_temp = BIO_new(BIO_s_mem());
    RSA *remote_pri_rsa = nullptr;

    // message
    char pub_en[1024];
    char pri_de[1024];
    char text[] = "1234567890";
    std::string ret;


    /**
     *
     */
    int res = BN_set_word(rsa_3, RSA_3);
    if(res != 1) {
        cout << "BN_set_word rsa RSA_3 error " << endl;
        goto error;
    }


    res = RSA_generate_key_ex(rsa, 1024, rsa_3, nullptr);
    if(res != 1) {
        cout << "RSA_generate_key_ex error " << endl;
        goto error;
    }

    pub_rsa = RSAPublicKey_dup(rsa);
    pri_rsa = RSAPrivateKey_dup(rsa);

    /**
     *
     */
#ifndef PASSWORD
    // rsa -> pem
    res = PEM_write_bio_RSAPrivateKey(bio_mem_temp, pri_rsa, nullptr, nullptr, 0, nullptr, nullptr);
    if(res != 1) {
        cout << "PEM_write_bio_RSAPrivateKey error " << endl;
        goto error;
    }
    // pem -> rsa
    remote_pri_rsa = PEM_read_bio_RSAPrivateKey(bio_mem_temp, nullptr, nullptr, nullptr);
    BIO_read(bio_mem_temp, cout_text, 5024);
    // cout << "bio_mem_temp:~" << cout_text << "~" << endl;
#endif

    /**
     *
     */
    // base 64 encode
    ret.resize((5024 + 2) / 3 * 4);
    EVP_EncodeBlock((unsigned char*)ret.data(), (unsigned char*)cout_text, 5024);
    cout << "after base64:" << ret << endl;


    /**
     *
     */

    // 公钥加密
    res = RSA_public_encrypt(
            strlen(text), (unsigned char *)text, (unsigned char *)pub_en, pub_rsa, RSA_PKCS1_OAEP_PADDING);
    if(res == -1) {
        cout << "RSA_public_encrypt error " << endl;
        goto error;
    }

    // 私钥解密
    res = RSA_private_decrypt(
            res, (unsigned char *)pub_en, (unsigned char *)pri_de, remote_pri_rsa, RSA_PKCS1_OAEP_PADDING);
    if(res == -1) {
        cout << "RSA_private_decrypt error " << endl;
        goto error;
    }

    cout << pri_de << endl;


error:
    RSA_free(rsa);
    BN_free(rsa_3);
    RSA_free(pub_rsa);
    RSA_free(pri_rsa);

    BIO_free(bio_mem_temp);
    RSA_free(remote_pri_rsa);
}