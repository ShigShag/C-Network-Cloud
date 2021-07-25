#ifndef CRYPTO_H
#define CRYPTO_H

#define PBKDF2_ITERATIONS 100000

/* Client ID generation */
int get_random_unsigned_long(unsigned long *r);
unsigned char *create_client_password_hash(char *pw, unsigned int count, unsigned char **salt);

#endif