#ifndef CLIENT_DATABASE_H
#define CLIENT_DATABASE_H

#include "Server.h"

/* Check if client is in database */
int Client_In_Database(Server *s, unsigned long id_);

/* Check if directory is in database */
int Directory_In_Database(Server *s, unsigned long directory_);

/* Get directory of client id */
char *Get_Client_Directory_Char(Server *s, unsigned long id_);
unsigned long Get_Client_Directory(Server *s, unsigned long id_);

/* Add client to Database */
int Add_Client_To_Database(Server *s, unsigned long id, unsigned long directory);

/* Create directory for client */
int Create_Client_Directory(Server *s, unsigned long directory);

#endif