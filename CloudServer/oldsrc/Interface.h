#ifndef INTERFACE_H
#define INTERFACE_H

#include <ncurses.h>
#include <pthread.h>
#include "Client.h"

typedef struct
{
    /* Server to create an interface for */
    pthread_t MainThread;

    /* If set to 0 end Thread */
    int active;

    /* Reference to servers client list */
    Client **client_list;
    int *clients_connected;

} Interface;

Interface *CreateInterface();  
void DeleteInterface(Interface *i);

void *InterfaceMainRoutine(void *i_);

#endif