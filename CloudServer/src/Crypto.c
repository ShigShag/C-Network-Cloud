#include "../inc/Crypto.h"
#include "../inc/Communication.h"
#include "../inc/ClientDatabase.h"
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/evp.h>


/* Generate random unsigned long */
int get_random_unsigned_long(unsigned long *r)
{
    return RAND_bytes((unsigned char *) r, sizeof(unsigned long));
}

/* Create new password hash */
unsigned char *create_client_password_hash(unsigned char *pw, unsigned int pw_length, unsigned char **salt)
{
    if(pw == NULL || pw_length == 0 || salt == NULL) return NULL;
    
    int salt_len = CLIENT_DATABASE_SALT_SIZE;
    *salt = malloc(salt_len);
    if(*salt == NULL) return NULL;

    if(RAND_bytes(*salt, salt_len) == 0){
        printf("[-] Could not allocate space for client password hash salt: %s\n", strerror(errno));
        free(*salt);
        return NULL;
    }

    unsigned char *hash = (unsigned char *) malloc(SHA512_DIGEST_LENGTH * sizeof(uint8_t));
    if(hash == NULL){
        printf("[-] Could not allocate space for client password hash: %s\n", strerror(errno));
        free(*salt);
        return NULL;
    }

    if(PKCS5_PBKDF2_HMAC((const char *) pw, pw_length, (const unsigned char * ) (*salt), salt_len, PBKDF2_ITERATIONS, EVP_sha512(), SHA512_DIGEST_LENGTH * sizeof(uint8_t), hash) == 0){
        free(*salt);
        free(hash);
        return NULL;
    }

    return hash;
}

/* Get a password hash from an existing salt */
unsigned char *get_client_password_hash(unsigned char *pw, unsigned int pw_length, unsigned char *salt)
{
    if(pw == NULL || pw_length == 0 || salt == NULL) return NULL;

    unsigned char *hash = (unsigned char *) malloc(SHA512_DIGEST_LENGTH * sizeof(uint8_t));
    if(hash == NULL){
        printf("[-] Could not allocate space for client password hash: %s\n", strerror(errno));
        return NULL;
    }

    if(PKCS5_PBKDF2_HMAC((const char *) pw, pw_length, *salt, CLIENT_DATABASE_SALT_SIZE, PBKDF2_ITERATIONS, EVP_sha512(), SHA512_DIGEST_LENGTH * sizeof(uint8_t), hash) == 0){
        free(hash);
        return NULL;
    }

    return hash;
}