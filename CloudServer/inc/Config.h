#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    /* socket */
    int port;
    int receive_timeout;

    /* server */
    int max_clients;
    
    /* Cloud node location */
    char *cloud_directory;
    
    /* Client database path */
    char *client_database_path;

    /* Client credentials path */
    char *client_credentials_path;

    /* Server log path */
    char *server_log_path;
    
    /* Client log path */
    char *client_log_directory;
} Config;

Config *Get_Config(char *f_name);
void Delete_Config(Config *c);

#endif