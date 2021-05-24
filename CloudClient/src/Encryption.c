#include "../inc/Encryption.h"
#include <openssl/err.h>

int Generate_Hash(void *source, unsigned long source_size, const EVP_MD *mode, unsigned char **destination, unsigned int *size)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    
    if(ctx != NULL)
    {
        if(EVP_DigestInit_ex(ctx, mode, NULL) == 1)
        {
            if(EVP_DigestUpdate(ctx, source, source_size) == 1)
            {
                if((*destination = (unsigned char *) calloc(EVP_MAX_MD_SIZE, sizeof(unsigned char))) != NULL)
                {
                    if(EVP_DigestFinal_ex(ctx, *destination, size))
                    {
                        EVP_MD_CTX_free(ctx);
                        return 1;
                    }
                }
            }
        }
        EVP_MD_CTX_free(ctx);
    }
    ERR_print_errors_fp(stderr);
    return 0;
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len = 0;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        return 0;

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
        return 0;

    if(EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1)
        return 0;
    ciphertext_len = len;

    if(EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
        return 0;
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len = 0;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        return 0;

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1)
        return 0;

    if(EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1)
        return 0;
    plaintext_len = len;

    if(EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1)
        return 0;
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}