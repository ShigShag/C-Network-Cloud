#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    /* socket */
    int port;
    int receive_timeout;

    /* server */
    int mode;
    int max_clients;
    
    /* Cloud node location */
    char *cloud_directory;
    
    /* Client database path */
    char *client_database_path;
} Config;

Config *Get_Config(char *f_name);

#endif