#include "../inc/ClientDatabase.h"
#include "../inc/Misc.h"
#include "../inc/Communication.h"

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
int Add_Client_credentials(Server *s, unsigned long id, unsigned char *pw, unsigned char *salt)
{
    if(s == NULL || pw == NULL || salt == NULL) return 0;

    FILE *fp;
    unsigned int count;
    unsigned char *formatted;
    unsigned long bytes_written;

    fp = fopen(s->config->client_credentials_path, "ab");
    if(fp == NULL){
        printf("[-] Failed to access %s: %s\n",s->config->client_credentials_path, strerror(errno));
        return 0;
    }

    formatted = Format_Client_Credentials(id, pw, salt, &count);
    if(formatted == NULL){
        fclose(fp);
        return 0;
    }

    bytes_written = fwrite(formatted, sizeof(unsigned char), count, fp);

    free(formatted);
    fclose(fp);
    return bytes_written != 0;
}   
unsigned char *Format_Client_Credentials(unsigned long id, unsigned char *pw, unsigned char *salt, unsigned int *count)
{
    if(pw == NULL || salt == NULL) return NULL;

    unsigned char *r = (unsigned char *) calloc(CLIENT_DATABASE_TOTAL_ENTRY_SIZE, sizeof(unsigned char));
    if(r == NULL)
    {
        printf("[-] Could not allocate memory for Format_Client_Credentials: %s\n", strerror(errno));
        return NULL;
    }

    unsigned char *id_uint8 = Uint64ToUint8(id);
    memcpy(r, id_uint8, CLIENT_ID_SIZE * sizeof(unsigned char));
    memcpy(r + (CLIENT_ID_SIZE * sizeof(unsigned char)), pw, CLIENT_DATABASE_PASSWORD_SIZE * sizeof(unsigned char));
    memcpy(r + ((CLIENT_ID_SIZE + CLIENT_DATABASE_PASSWORD_SIZE) * sizeof(unsigned char)), salt, CLIENT_DATABASE_SALT_SIZE * sizeof(unsigned char));

    free(id_uint8);
    *count = (CLIENT_DATABASE_TOTAL_ENTRY_SIZE) * sizeof(unsigned char);
    return r;
}


/* Check password hash for a client id */
int Check_Client_Password(Server *s, unsigned long id, unsigned char *pw)
{
    if(s == NULL || pw == NULL) return 0;

    FILE *fp = fopen(s->config->client_credentials_path, "rb");
    if(fp == NULL){
        printf("[-] Failed to access %s: %s\n",s->config->client_credentials_path, strerror(errno));
        return 0;
    }

    // Hier weiter machen


}
