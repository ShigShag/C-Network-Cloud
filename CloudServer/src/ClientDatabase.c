#include "../inc/ClientDatabase.h"
#include "../inc/Misc.h"

/* Check if client is in database */
int Client_In_Database(Server *s, int id)
{
    FILE *fp;
    char line[512];
    char directory[128];
    int int_id = 0;
    int in_directory = 0;

    fp = fopen(s->config->client_database_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }

    fgets(line, sizeof(line), fp);
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%d %s", &int_id, directory);
        if(int_id == id)
        {
            in_directory = 1;
            break;
        }
    }
    fclose(fp);
    return in_directory;
}

/* Get directory of client id */
char *Get_Client_Directory(Server *s, int id)
{
    FILE *fp;
    char line[512];
    char directory[128];
    char *return_directroy = NULL;
    int int_id = 0;

    fp = fopen(s->config->client_database_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",s->config->client_database_path, strerror(errno));
        return 0;
    }

    fgets(line, sizeof(line), fp);
    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%d %s", &int_id, directory);
        if(int_id == id)
        {
            return_directroy = (char *) malloc((strlen(directory)  + 1) * sizeof(char));
            if(return_directroy == NULL)
            {
                printf("[-] Failed to allocate space for return_directroy: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(return_directroy, directory, strlen(directory) + 1);
            break;
        }
    }
    fclose(fp);
    return return_directroy;
}

/* Add client to Database */
int Add_Client_To_Database(Server *s, int id, char *directory)
{
    if(s == NULL || directory == NULL) return 0;

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
    err = fprintf(fp, "%d %s\n", id, directory);
    fclose(fp);

    Create_Client_Directory(s, directory);

    return err > 0;
}
int Create_Client_Directory(Server *s, char *directory)
{
    if(s == NULL || directory == NULL) return 0;

    char *dir = append_malloc(s->config->cloud_directory, directory);
    if(dir == NULL)
    {
        printf("[-] Could not allocate space for append dir in add_client_to_database: %s\n", strerror(errno));
        return 0;    
    }

    mkdir(dir, 0755);
    free(dir);
    return 1;
}