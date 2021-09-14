#ifndef IDENTITY_H
#define IDENTITY_H

#include "ClientBackend.h"

unsigned long Get_Client_Id(Client *c);
int Set_Client_Id(Client *c, unsigned long id);
void Create_Client_Id_File(Client *c);

#endif