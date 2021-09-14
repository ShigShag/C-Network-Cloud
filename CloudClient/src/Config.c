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

    c->identity_path = NULL;
    c->ip = NULL;
    c->port = 0;
    c->receive_timeout = 0;

    fp = fopen(f_name, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",f_name, strerror(errno));
        return NULL;
    }

    while(fgets(line, sizeof(line), fp))
    {
        sscanf(line,"%s %s", var, value);

        if(!strcmp(var, "port") && strlen(value) > 0){
            c->port = strtol(value, NULL, 10);
        }else if(!strcmp(var, "receive_timeout") && strlen(value) > 0){
            c->receive_timeout = strtol(value, NULL, 10);
        }else if(!strcmp(var, "ip") && strlen(value) > 0){
            c->ip = (char *) malloc(strlen(value) + 1);
            if(c->ip == NULL){
                printf("[-] Failed to allocate space for c->identity_path: %s\n", strerror(errno));
                return NULL;
            }
            strncpy(c->ip, value, strlen(value) + 1);
        }else if(!strcmp(var, "identity_path") && strlen(value) > 0){
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

    int passed = 1;

    if(c->identity_path == NULL){
        fprintf(stderr, "[-] identity_path was not set -> Usage example: identity_path path/identity.txt\n");
        passed = 0;
    }
    if(c->ip == NULL){
        fprintf(stderr, "[-] Ip was not set -> Usage example: ip 127.0.0.1\n");
        passed = 0;
    }
    if(c->port == 0){
        fprintf(stderr, "[-] port was not set -> Usage example: port 8080\n");
        passed = 0;
    }
    if(passed == 0){
        Delete_Config(c);
        fprintf(stderr, "exiting...\n");
        c = NULL;
    }

    fclose(fp);
    return c;
}
void Delete_Config(Config *c)
{
    if(c == NULL) return;

    if(c->identity_path != NULL) free(c->identity_path);
    if(c->ip != NULL) free(c->ip);

    free(c);
}