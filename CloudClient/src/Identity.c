#include "../inc/Identity.h"
#include "../inc/Misc.h"

unsigned long Get_Client_Id(Client *c)
{
    if(c == NULL) return 0;

    FILE *fp;
    unsigned long id = 0;

    fp = fopen(c->config->identity_path, "r");
    if(fp == NULL)
    {
        printf("[-] Failed to access %s: %s\n",c->config->identity_path, strerror(errno));
        return 0;
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
void Create_Client_Id_File(Client *c)
{
    if(c == NULL) return;

    printf("[+] Creating new identity file\n");

    char *dir = get_directory_name(c->config->identity_path);
    if(dir != NULL)
    {
        mkdir_recursive(dir, 0700);
        printf("[+] Created directory: [%s]\n", dir);
        free(dir);
    }

    int fd = open(c->config->identity_path, O_CREAT, S_IRUSR | S_IWUSR);
    printf("[+] Created file: [%s]\n", c->config->identity_path);
    close(fd);
}