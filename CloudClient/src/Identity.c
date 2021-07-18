#include  "../inc/Identity.h"

unsigned long Get_Client_Id(Client *c)
{
    if(c == NULL) return -1;

    FILE *fp;
    unsigned long id = 0;

    fp = fopen(c->config->identity_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",c->config->identity_path, strerror(errno));
        return -1;
    }

    fscanf(fp, "%lu", &id);

    fclose(fp);
    return id;
}
int Set_Client_Id(Client *c, unsigned long id)
{
    FILE *fp;
    int n;

    fp = fopen(c->config->identity_path, "w");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",c->config->identity_path, strerror(errno));
        return 0;
    }

    n = fprintf(fp, "%lu", id);
    fclose(fp);
    return n;
}