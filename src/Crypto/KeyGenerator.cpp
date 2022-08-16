#include "KeyGenerator.h"

std::tuple<std::string, std::string> GenerateKey()
{
    std::string key_str;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_keygen(pctx, &pkey);

    BIO* bio_private = BIO_new(BIO_s_mem());
    auto ret = PEM_write_bio_PrivateKey(bio_private, pkey, NULL, NULL, 0, NULL, NULL);
    if (ret != 1)
    {
        std::cout << "ERROR! "<< std::endl;
    }
    BIO_flush(bio_private);

    BIO* bio_public = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio_public, pkey);

    char* priv_c_str;
    BIO_get_mem_data(bio_private, &priv_c_str);
    std::string private_key_txt(priv_c_str);

    char* pub_c_str;
    BIO_get_mem_data(bio_public, &pub_c_str);
    std::string public_key_txt(pub_c_str);

    // ToDo Check error codes on openssl API

    BIO_free(bio_private);
    BIO_free(bio_public);

    EVP_PKEY_CTX_free(pctx);

    return std::tuple<std::string, std::string>(private_key_txt, public_key_txt);
}