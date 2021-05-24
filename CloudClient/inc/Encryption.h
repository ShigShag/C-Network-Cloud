#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <openssl/evp.h>

/* Hashing */
int Generate_Hash(void *source, unsigned long source_size, const EVP_MD *mode, unsigned char **destination, unsigned int *size);

/* Asymetric Encryption */

/* Aes encryption using cbc -> do not use this for big data encryption */
int Aes_Encrypt(unsigned char *source, unsigned long source_size, unsigned char *key, unsigned char *iv, unsigned char **destination, unsigned long *destination_size);
 
#endif