#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <linux/limits.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#include "Logging.h"

typedef struct
{
    /* Client id */
    unsigned long id;

    /* Clients directory represtend as an unsigned long */
    unsigned long directory;

    /* Thread for main routine */
    pthread_t h_client;

    /* Socket */
    int socket;
    struct sockaddr_in addr;
    int port;
    char *ip;

    /* Fast transmission clients */
    int transmission_client_allowed;
    int transmission_client_count;
    int *transmission_client_array;

    /* Status */
    int Active;
    int test_mode;

    /* Encryption */
    unsigned char *aes_key;

    /* Cloud directory */
    char *complete_cloud_directory;

    /* Logger */
    Log *log;

} Client;

typedef struct
{
    /* socket */
    int in;

    /* File to write */
    FILE *out;

} RECV_ARG;

void *ClientRoutine(void *client);

/* Receive file from client */
void Receive_File(Client *c, char *f_name);

/* Send file to client */
void Send_File(Client *c, char *f_name);

/* List files to client */
void List_File(Client *c);

/* Deletes a file */
void Delete_File(Client *c, char *f_name);

/* Cat a file */
void Cat_File(Client *c, char *f_name);

#endif