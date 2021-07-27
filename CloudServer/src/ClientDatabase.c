#include "../inc/ClientDatabase.h"
#include "../inc/Misc.h"
#include "../inc/Communication.h"
#include "../inc/Crypto.h"
#include <openssl/crypto.h>
#include <openssl/rand.h>

/* --------- client_database.txt --------- */

/* Check if client is in database */
int Client_In_Database(Server *s, unsigned long id_)
{
    FILE *fp;
    char line[CLIENT_ID_SIZE + (NAME_MAX + 1) + 10];
    unsigned long directory;
    unsigned long id = 0;
    int in_database = 0;

    fp = fopen(s->config->client_database_path, "rb");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }

    fgets(line, sizeof(line), fp);
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%lu %lu", &id, &directory);
        if(id == id_)
        {
            in_database = 1;
            break;
        }
    }
    fclose(fp);
    return in_database;
}
/* Check if directory is in database */
int Directory_In_Database(Server *s, unsigned long directory_)
{
    FILE *fp;
    char line[CLIENT_ID_SIZE + (NAME_MAX + 1) + 10];
    unsigned long directory;
    unsigned long id = 0;
    int in_database = 0;

    fp = fopen(s->config->client_database_path, "rb");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }

    fgets(line, sizeof(line), fp);
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%lu %lu", &id, &directory);
        if(directory == directory_)
        {
            in_database = 1;
            break;
        }
    }
    fclose(fp);
    return in_database;
}
/* Get directory of client id */
char *Get_Client_Directory_Char(Server *s, unsigned long id_)
{
    FILE *fp;
    char line[CLIENT_ID_SIZE + (NAME_MAX + 1) + 10];
    unsigned long directory;
    char *return_directroy = NULL;
    unsigned long id = 0;

    fp = fopen(s->config->client_database_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }

    fgets(line, sizeof(line), fp);
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%lu %lu", &id, &directory);
        if(id == id_)
        {
            return_directroy = (char *) malloc((NAME_MAX + 1) * sizeof(char));
            if(return_directroy == NULL)
            {
                printf("[-] Failed to allocate space for return_directroy: %s\n", strerror(errno));
                return NULL;
            }
            snprintf(return_directroy, (NAME_MAX + 1) * sizeof(char), "%lu", directory);
            break;
        }
    }
    fclose(fp);
    return return_directroy;
}
unsigned long Get_Client_Directory(Server *s, unsigned long id_)
{
    FILE *fp;
    char line[CLIENT_ID_SIZE + (NAME_MAX + 1) + 10];
    unsigned long directory = 0;
    unsigned long id = 0;

    fp = fopen(s->config->client_database_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }

    fgets(line, sizeof(line), fp);
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%lu %lu", &id, &directory);
        if(id == id_) break;
    }
    fclose(fp);
    return directory;
}
/* Add client to Database */
int Add_Client_To_Database(Server *s, unsigned long id, unsigned long directory)
{
    if(s == NULL) return 0;

    FILE *fp;
    int err;
    
    if(Client_In_Database(s, id) == 1) return 0;

    // Add client to database
    fp = fopen(s->config->client_database_path, "a");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }
    err = fprintf(fp, "%lu %lu\n", id, directory);
    fclose(fp);

    Create_Client_Directory(s, directory);

    return err > 0;
}
int Create_Client_Directory(Server *s, unsigned long directory)
{
    if(s == NULL) return 0;

    char temp[NAME_MAX + 1];
    snprintf(temp, sizeof(temp), "%lu", directory);
    char *dir = append_malloc(s->config->cloud_directory, temp);
    if(dir == NULL)
    {
        printf("[-] Could not allocate space for append dir in add_client_to_database: %s\n", strerror(errno));
        return 0;    
    }

    mkdir(dir, 0755);
    free(dir);
    return 1;
}

/* --------- credentials.txt --------- */

/* Add clients credentials to the database */
/* Should only be called from within the ClientDatabase.c */
int Add_Client_credentials(Server *s, unsigned long id, char *pw)
{
    if(s == NULL || pw == NULL) return 0;

    FILE *fp;
    unsigned int count;
    unsigned char *formatted;
    unsigned char *pw_hash;
    unsigned char *salt;
    unsigned long bytes_written;

    fp = fopen(s->config->client_credentials_path, "ab");
    if(fp == NULL){
        printf("[-] Failed to access %s: %s\n",s->config->client_credentials_path, strerror(errno));
        return 0;
    }

    salt = (unsigned char *) malloc(CLIENT_DATABASE_SALT_SIZE);
    if(salt == NULL){
        printf("[-] Failed to allocate memory for salt: %s\n", strerror(errno));
        fclose(fp);
        return 0;
    }

    if(RAND_bytes(salt, CLIENT_DATABASE_SALT_SIZE) == 0){
        fclose(fp);
        free(salt);
        return 0;
    }

    // Create password hash
    pw_hash = get_client_password_hash(pw, strlen(pw), salt);
    if(pw_hash == NULL){
        fclose(fp);
        return 0;
    }

    // Format data into one byte string
    formatted = Format_Client_Credentials(id, pw_hash, salt, &count);
    if(formatted == NULL){
        fclose(fp);
        free_memset(pw_hash, CLIENT_DATABASE_PASSWORD_HASH_SIZE);
        return 0;
    }

    // Write byte string to the end of the file
    bytes_written = fwrite(formatted, sizeof(unsigned char), count, fp);

    free(formatted);
    fclose(fp);
    free_memset(pw_hash, CLIENT_DATABASE_PASSWORD_HASH_SIZE);
    free_memset(salt, CLIENT_DATABASE_SALT_SIZE);
    return bytes_written != 0;
}   
unsigned char *Format_Client_Credentials(unsigned long id, unsigned char *pw_hash, unsigned char *salt, unsigned int *count)
{
    if(pw_hash == NULL || salt == NULL) return NULL;

    unsigned char *r = (unsigned char *) calloc(CLIENT_DATABASE_TOTAL_ENTRY_SIZE, sizeof(unsigned char));
    if(r == NULL)
    {
        printf("[-] Could not allocate memory for Format_Client_Credentials: %s\n", strerror(errno));
        return NULL;
    }

    unsigned char *id_uint8 = Uint64ToUint8(id);
    memcpy(r, id_uint8, CLIENT_ID_SIZE * sizeof(unsigned char));
    memcpy(r + (CLIENT_ID_SIZE * sizeof(unsigned char)), pw_hash, CLIENT_DATABASE_PASSWORD_HASH_SIZE * sizeof(unsigned char));
    memcpy(r + ((CLIENT_ID_SIZE + CLIENT_DATABASE_PASSWORD_HASH_SIZE) * sizeof(unsigned char)), salt, CLIENT_DATABASE_SALT_SIZE * sizeof(unsigned char));

    free(id_uint8);
    *count = (CLIENT_DATABASE_TOTAL_ENTRY_SIZE) * sizeof(unsigned char);
    return r;
}

/* Get clients credentials from formatted bytes */
/* NOTE: THIS FUNCTION ASSUMES THAT pw AND salt ARE ALREADY ALLOCATED */
int Get_Client_Credentials(unsigned char *formatted, Database_Client *dc)
{
    if(formatted == NULL) return 0;

    dc->id = Uint8ToUint64(formatted);
    memcpy(dc->pw_hash, formatted + CLIENT_ID_SIZE, CLIENT_DATABASE_PASSWORD_HASH_SIZE);
    memcpy(dc->salt, formatted + CLIENT_ID_SIZE + CLIENT_DATABASE_PASSWORD_HASH_SIZE, CLIENT_DATABASE_SALT_SIZE);
    return 1;
}

/* Get clients salt */
unsigned char *Get_Client_Salt(Server *s, unsigned long id)
{
    if(s == NULL) return NULL;

    unsigned char *salt = (unsigned char *) malloc(CLIENT_DATABASE_SALT_SIZE * sizeof(unsigned char));
    if(salt == NULL){
        printf("[-] Could not allocate space for salt in Get_Client_Salt: %s\n", strerror(errno));
        return NULL;
    }
    
    unsigned char *client_credentials = (unsigned char *) malloc(CLIENT_DATABASE_TOTAL_ENTRY_SIZE * sizeof(unsigned char));
    if(client_credentials == NULL){
        printf("[-] Could not allocate space for client_credentials in Get_Client_Salt: %s\n", strerror(errno));
        free(salt);
        return NULL;
    }

    Database_Client *dc = (Database_Client *) malloc(sizeof(Database_Client));
    if(dc == NULL){
        printf("[-] Could not allocate space for Database_Client in Get_Client_Salt: %s\n", strerror(errno));
        free(salt);
        free(client_credentials);
        return NULL;
    }

    unsigned long err = 1;
    int success = 0;

    FILE *fp = fopen(s->config->client_credentials_path, "rb");
    if(fp == NULL){
        printf("[-] Failed to access %s: %s\n",s->config->client_credentials_path, strerror(errno));
        return 0;
    }

    while(err != 0)
    {
        err = fread(client_credentials, sizeof(unsigned char), CLIENT_DATABASE_TOTAL_ENTRY_SIZE, fp);
        if(err == 0) continue;

        Get_Client_Credentials(client_credentials, dc);

        if(dc->id == id){
            success = 1;
            memcpy(salt, dc->salt, CLIENT_DATABASE_SALT_SIZE);
            break;
        }
    }

    if(success != 1){
        free_memset(salt, CLIENT_DATABASE_SALT_SIZE);
        salt = NULL;
    }

    free_memset(client_credentials, CLIENT_DATABASE_TOTAL_ENTRY_SIZE);
    free_memset(dc, sizeof(Database_Client));

    return salt;
}

/* Check password hash for a client id */
int Check_Client_Password(Server *s, unsigned long id, char *pw)
{
    if(s == NULL || pw == NULL) return 0;

    Database_Client *dc = (Database_Client *) malloc(sizeof(Database_Client));
    if(dc == NULL){
        printf("[-] Could not allocate space for Database_Client in Check_Client_Password: %s\n", strerror(errno));
        return 0;
    }

    unsigned char *client_credentials = (unsigned char *) malloc(CLIENT_DATABASE_TOTAL_ENTRY_SIZE * sizeof(unsigned char));
    if(client_credentials == NULL){
        printf("[-] Could not allocate space for client_credentials in Check_Client_Password: %s\n", strerror(errno));
        free(dc);
        return 0;
    }

    FILE *fp = fopen(s->config->client_credentials_path, "rb");
    if(fp == NULL){
        printf("[-] Failed to access %s: %s\n",s->config->client_credentials_path, strerror(errno));
        return 0;
    }

    int equal = 1;
    unsigned long err = 1;

    unsigned char *pw_hash;

    while(err != 0)
    {
        err = fread(client_credentials, sizeof(unsigned char), CLIENT_DATABASE_TOTAL_ENTRY_SIZE, fp);
        if(err == 0) continue;

        Get_Client_Credentials(client_credentials, dc);

        if(dc->id == id){
            pw_hash = get_client_password_hash(pw, strlen(pw), dc->salt);
            if(pw_hash == NULL) break;
        
            equal = CRYPTO_memcmp(pw_hash, dc->pw_hash, CLIENT_DATABASE_PASSWORD_HASH_SIZE);
            free(pw_hash);
            break;
        }
    }
    free_memset(client_credentials, CLIENT_DATABASE_TOTAL_ENTRY_SIZE);
    
    fclose(fp);
    return equal == 0;
}