#ifndef CLIENT_DATABASE_H
#define CLIENT_DATABASE_H

#include "Server.h"

/* Check if client is in database */
int Client_In_Database(Server *s, int id);

/* Get directory of client id */
char *Get_Client_Directory(Server *s, int id);

/* Add client to Database */
int Add_Client_To_Database(Server *s, int id, char *directory);

/* Create directory for client */
int Create_Client_Directory(Server *s, char *directory);

#endif