#ifndef CLIENT_MANAGMENT_H
#define CLIENT_MANAGMENT_H

#include "ClientDatabase.h"

/* Listening */
void *ClientListeningThread(void *server);

/* Monitor */
void *ClientListMonitor(void *server);

/* Management  */
void ManageClient(int socket, Server *s);

/* Add active client in memory */
Client *CreateClient(Server *s, int socket, unsigned int id);

/* Disposal */
void RemoveClient(Server *s, int index);
void RemoveAllClients(Server *s);
void ReportDisconnect(Client *client);

/* Thread locks */
int Lock_Client_Count(Server *s);
int Unlock_Client_Count(Server *s);

#endif