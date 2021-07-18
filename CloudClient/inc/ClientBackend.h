#ifndef CLIENT_BACKEND_H
#define CLIENT_BACKEND_H

#include "Interface.h"
#include "ProgressBar.h"
#include "Config.h"

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

typedef struct
{
    /* Socket */
    int socket;
    struct sockaddr_in addr;
    int Active;
    int is_connected;

    /* id */
    unsigned long id;

    /* Encryption */
    unsigned char *aes_key;
    
    /* Config */
    Config *config;
    
}  Client;

typedef struct
{
    Client *client;
    Interface *interface;
} Interface_arg;

/* transmission client */
typedef struct 
{
    /* socket */
    int socket;
    struct sockaddr_in addr;
    int is_connected;

    /* Parent client id */
    unsigned long p_id;

    /* thread */
    pthread_t thread;
} client_t;

/* Transmission client arguments */
typedef struct
{
    /* File descriptor */
    int in;

    /* Socket */
    int out;

    /* File offset */
    off_t offset;

    /* Count of bytes to send over socket */
    int count;
} SEND_ARG;

typedef struct
{
    /* socket */
    int in;

    /* File to write */
    FILE *out;

} RECV_ARG;

/* Client initialisation */
Client *Create_Client(char *config_file_path);

/* Transmission clients */
client_t *Create_Transmission_Client(char *ip, int port, unsigned long p_id);
int Connect_Transmission_Client(client_t *c);
int Initial_Handshake_t(client_t *c);
void *Send_Packet_t(void *arg);
void Delete_Transmission_Client(client_t *c);

/* Server connection */
int Connect_To_Server(Client *c);
int Initial_Handshake(Client *c);

/* Back end routine */
//void *Main_Routine_Back_End(Client *c, Interface *i);
void *Main_Routine_Back_End(void *arg);

/* Input translation */
int Translate_Input(char *input);

/* File transmition */
void Push_File(Client *c, Interface *i);

void Push_File_f(Client *c, Interface *i);

/* File pulling */
void Pull_File(Client *c, Interface *i);

/* File listing */
void List_File(Client *c, Interface *i);

/* File deletion */
void Delete_File(Client *c, Interface *i);

/* File cat */
void Cat_File(Client *c, Interface *i);

/* Help */
void Help(Interface *i);

/* Clear Terminal */
void Clear_Terminal();

/* Client quit */
void ReportDisconnect(Client *c);
void Delete_Client(Client *c);

#endif