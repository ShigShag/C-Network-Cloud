#include "../inc/Config.h"
#include "../inc/Server.h"


Config *Get_Config(char *f_name)
{
    FILE *fp;
    char line [512];
    char var [128];
    char value [128];

    Config *c = (Config *) malloc(sizeof(Config));
    if(c == NULL)
    {
        printf("[-] Failed to allocate space for Config: %s\n", strerror(errno));
        return NULL;
    }

    /* Minimum requirements */
    c->port = 0;
    c->client_credentials_path = NULL;
    c->client_database_path = NULL;
    c->cloud_directory = NULL;
    c->client_log_directory = NULL;
    c->server_log_path = NULL;

    /* Optional */
    c->max_clients = 10;
    c->receive_timeout = 0;

    fp = fopen(f_name, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",f_name, strerror(errno));
        return NULL;
    }

    while(fgets(line, sizeof(line), fp))
    {
        memset(value, 0, sizeof(value));
        sscanf(line,"%s %s", var, value);

        if(!strcmp(var, "port") && strlen(value) > 0){
            c->port = strtol(value, NULL, 10);
        }else if(!strcmp(var, "receive_timeout") && strlen(value) > 0){
            c->receive_timeout = strtol(value, NULL, 10); 
        }else if(!strcmp(var, "max_clients") && strlen(value) > 0){
            c->max_clients = strtol(value, NULL, 10);
        }else if(!strcmp(var, "receive_timeout") && strlen(value) > 0){
            c->receive_timeout = strtol(value, NULL, 10);
        }else if(!strcmp(var, "cloud_directory") && strlen(value) > 0){
            c->cloud_directory = (char *) malloc(strlen(value) + 1);
            if(c->cloud_directory == NULL){
                printf("[-] Failed to allocate space for c->cloud_directory: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->cloud_directory, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "client_database_path") && strlen(value) > 0){
            c->client_database_path = (char *) malloc(strlen(value) + 1);
            if(c->client_database_path == NULL){
                printf("[-] Failed to allocate space for c->client_database_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->client_database_path, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "client_credentials_path") && strlen(value) > 0){
            c->client_credentials_path = (char *) malloc(strlen(value) +  1);
            if(c->client_credentials_path == NULL){
                printf("[-] Failed to allocate space for c->client_credentials_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->client_credentials_path, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "server_log_path") && strlen(value) > 0){
            c->server_log_path = (char *) malloc(strlen(value) +  1);
            if(c->server_log_path == NULL){
                printf("[-] Failed to allocate space for c->server_log_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->server_log_path, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "client_log_directory") && strlen(value) > 0){
            c->client_log_directory = (char *) malloc(strlen(value) +  1);
            if(c->client_log_directory == NULL){
                printf("[-] Failed to allocate space for c->client_log_directory: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->client_log_directory, value, strlen(value) + 1);
        }
        else{
            printf("[!] Unknown or uncomplete Parameter in config file: %s\n", var);
        }
    }

    int passed = 1;

    if(c->port == 0){
        fprintf(stderr, "[-] port was not set -> Usage example: port 8080\n");
        passed = 0;
    }
    if(c->client_credentials_path == NULL){
        fprintf(stderr, "[-] client_credentials path was not set -> Usage example: client_credentials_path client_credentials/credentials.txt\n");
        passed = 0;
    }
    if(c->client_database_path == NULL){
        fprintf(stderr, "[-] client_database_path was not set -> Usage example: client_database_path client_database_path/database.txt\n");
        passed = 0;
    }
    if(c->cloud_directory == NULL){
        fprintf(stderr, "[-] cloud_directory was not set -> Usage example: cloud_directory cloud/\n");
        passed = 0;
    }
    if(c->server_log_path == NULL){
        fprintf(stderr, "[-] Server log path was not set -> Usage example: Logs/server.log\n");
    }
    if(c->client_log_directory == NULL){
        fprintf(stderr, "[-] Client log directory was not set -> Usage example: Client-Logs/\n");
    }

    if(passed == 0){
        Delete_Config(c);
        c = NULL;
        fprintf(stderr, "exiting...\n");
    }

    fclose(fp);
    return c;
}
void Delete_Config(Config *c)
{
    if(c == NULL) return;

    if(c->cloud_directory != NULL) free(c->cloud_directory);
    if(c->client_database_path != NULL) free(c->client_database_path);
    if(c->client_credentials_path != NULL) free(c->client_credentials_path);
    if(c->server_log_path != NULL) free(c->server_log_path);
    if(c->client_log_directory != NULL) free(c->client_log_directory);
    free(c);
}