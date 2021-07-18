#include "../inc/ClientDatabase.h"
#include "../inc/Misc.h"
#include "../inc/Communication.h"

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