#include "../inc/Config.h"
#include "../inc/ClientBackend.h"

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
        }else if(!strcmp(var, "ip")){
            c->ip = (char *) malloc(strlen(value) + 1);
            if(c->ip == NULL){
                printf("[-] Failed to allocate space for c->identity_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->ip, value, strlen(value) + 1);
        }else if(!strcmp(var, "identity_path")){
            c->identity_path = (char *) malloc(strlen(value) + 1);
            if(c->identity_path == NULL){
                printf("[-] Failed to allocate space for c->identity_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->identity_path, value, strlen(value) + 1);
        }
        else{
            printf("[!] Unknown Parameter in config file: %s\n", var);
        }
    }

    fclose(fp);
    return c;
}