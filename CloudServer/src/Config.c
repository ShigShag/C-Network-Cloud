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

    fp = fopen(f_name, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",f_name, strerror(errno));
        return NULL;
    }

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line,"%s %s", var, value);

        if(!strcmp(var, "port")){
            c->port = strtol(value, NULL, 10);
        }else if(!strcmp(var, "receive_timeout")){
            c->receive_timeout = strtol(value, NULL, 10);
        }else if(!strcmp(var, "mode")){
            c->mode = strtol(value, NULL, 10); 
        }else if(!strcmp(var, "max_clients")){
            c->max_clients = strtol(value, NULL, 10);
        }else if(!strcmp(var, "receive_timeout")){
            c->receive_timeout = strtol(value, NULL, 10);
        }else if(!strcmp(var, "cloud_directory")){
            c->cloud_directory = (char *) malloc(strlen(value) + 1);
            if(c->cloud_directory == NULL){
                printf("[-] Failed to allocate space for c->cloud_directory: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->cloud_directory, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "client_database_path")){
            c->client_database_path = (char *) malloc(strlen(value) + 1);
            if(c->cloud_directory == NULL){
                printf("[-] Failed to allocate space for c->client_database_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->client_database_path, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "client_credentials_path")){
            c->client_credentials_path = (char *) malloc(strlen(value) +  1);
            if(c->client_credentials_path == NULL){
                printf("[-] Failed to allocate space for c->client_credentials_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->client_credentials_path, value, strlen(value) + 1);
        }
        else if(!strcmp(var, "server_log_path")){
            c->server_log_path = (char *) malloc(strlen(value) +  1);
            if(c->server_log_path == NULL){
                printf("[-] Failed to allocate space for c->server_log_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->server_log_path, value, strlen(value) + 1);
        }
        else{
            printf("[!] Unknown Parameter in config file: %s\n", var);
        }
    }

    fclose(fp);
    return c;
}