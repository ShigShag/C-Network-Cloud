#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    /* socket */
    char *ip;
    int port;
    int receive_timeout;
    
    /* identity path location */
    char *identity_path;

} Config;

Config *Get_Config(char *f_name);

#endif