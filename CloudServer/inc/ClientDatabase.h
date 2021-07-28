#ifndef CLIENT_DATABASE_H
#define CLIENT_DATABASE_H

#include "Server.h"
#include <openssl/sha.h>

#define CLIENT_DATABASE_ID_SIZE sizeof(unsigned long)
#define CLIENT_DATABASE_PASSWORD_HASH_SIZE SHA512_DIGEST_LENGTH 
#define CLIENT_DATABASE_SALT_SIZE 16
#define CLIENT_DATABASE_TOTAL_ENTRY_SIZE (CLIENT_DATABASE_ID_SIZE + CLIENT_DATABASE_PASSWORD_HASH_SIZE + CLIENT_DATABASE_SALT_SIZE)

/* --------- client_database.txt --------- */

/* Check if client is in database */
int Client_In_Database(Server *s, unsigned long id_);

/* Check if directory is in database */
int Directory_In_Database(Server *s, unsigned long directory_);

/* Get directory of client id */
char *Get_Client_Directory_Char(Server *s, unsigned long id_);
unsigned long Get_Client_Directory(Server *s, unsigned long id_);

/* Add client to Database */
int Add_Client_To_Database(Server *s, unsigned long id, unsigned long directory);

/* Create directory for client */
int Create_Client_Directory(Server *s, unsigned long directory);

/* --------- credentials.txt --------- */

/* [CLIENT ID][CLIENT PW HASH][CLIENT SALT] */
/* [8 Byte   ][64 Byte       ][16 Bytes   ] */

typedef struct{
    unsigned long id;
    unsigned char pw_hash[CLIENT_DATABASE_PASSWORD_HASH_SIZE];
    unsigned char salt[CLIENT_DATABASE_SALT_SIZE];
} Database_Client;

/* Add clients credentials to the database */
int Add_Client_credentials(Server *s, unsigned long id, char *pw);

/* Get clients credentials formatted for the database */
unsigned char *Format_Client_Credentials(unsigned long id, unsigned char *pw_hash, unsigned char *salt, unsigned int *count);

/* Get clients credentials from formatted bytes */
/* Should only be called from within the ClientDatabase.c */
int Get_Client_Credentials(char *formatted, Database_Client *dc);

/* Get clients salt */
unsigned char *Get_Client_Salt(Server *s, unsigned long id);

/* Check password hash for a client id */
int Check_Client_Password(Server *s, unsigned long id, char *pw);

#endif