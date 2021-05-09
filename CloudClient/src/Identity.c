#include  "../inc/Identity.h"

int Get_Client_Id(Client *c)
{
    if(c == NULL) return -1;

    FILE *fp;
    int id = 0;

    fp = fopen(c->config->identity_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",c->config->identity_path, strerror(errno));
        return -1;
    }

    fscanf(fp, "%d", &id);

    fclose(fp);
    return id;
}
int Set_Client_Id(Client *c, int id)
{
    FILE *fp;
    int n;

    fp = fopen(c->config->identity_path, "w");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",c->config->identity_path, strerror(errno));
        return 0;
    }

    n = fprintf(fp, "%d", id);
    fclose(fp);
    return n;
}