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

typedef struct
{
    /* Client id */
    unsigned int id;

    /* Thread for main routine */
    pthread_t h_client;

    /* Socket */
    int socket;
    struct sockaddr_in addr;
    int port;
    char *ip;

    /* Status */
    int Active;
    int test_mode;

    /* Encryption */
    unsigned char *aes_key;

    /* Cloud directory */
    char *cloud_directory;
    char *server_cloud_directory;
} Client;

void *ClientRoutine(void *client);

/* Receive file from client */
void Receive_File(Client *c, char *f_name);

/* Send file to client */
void Send_File(Client *c);

/* List files to client */
void List_File(Client *c);

#endif