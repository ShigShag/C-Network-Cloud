#ifndef SERVER_H
#define SERVER_H

#include "Client.h"
#include "Config.h"

typedef struct
{
    /* Socket */
    int Socket;
    struct sockaddr_in addr;
    
    /* Threads */
    pthread_t ListeningThread;
    pthread_t ClientMonitorThread;
    
    /* Synchronisation */
    int Client_Lock;

    /* Status */
    int Activated;
    int max_clients;
    int clients_connected;
    
    /* Config */
    Config *config;
    int port;
    int mode;

    /* Client list */
    Client **CLIENT;

} Server;

/* Create Server from config file */
Server *Create_Server(char *config_file_path);

/* Create and bind socket */
int Create_Socket(Server *s, int receive_timeout_sec);

/* Start Servers main routine */
int StartServer(Server *s);

/* Delete Serer and clean everything up */
void DeleteServer(Server *s);

#endif