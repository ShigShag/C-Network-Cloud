#include "../inc/Crypto.h"
#include "../inc/Communication.h"
#include <openssl/rand.h>
#include <openssl/sha.h>


int get_random_unsigned_long(unsigned long *r)
{
    return RAND_bytes((unsigned char *) r, sizeof(unsigned long));
}