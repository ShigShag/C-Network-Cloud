#ifndef CRYPTO_H
#define CRYPTO_H

#define PBKDF2_ITERATIONS 100000

/* Generate random unsigned long */
int get_random_unsigned_long(unsigned long *r);

/* Create new password hash */
unsigned char *create_client_password_hash(char *pw, unsigned int pw_length, unsigned char **salt);

/* Get a password hash from an existing salt */
unsigned char *get_client_password_hash(char *pw, unsigned int pw_length, unsigned char *salt);

#endif