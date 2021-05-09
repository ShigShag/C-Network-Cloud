#include "../inc/ClientManagment.h"
#include "../inc/Communication.h"
#include "../inc/Misc.h"

/* Listening */
// Listens for new client and passes them to the manage function
void *ClientListeningThread(void *server)
{
    if(server == NULL) return 0;

    srand(time(NULL));

    Server *s = (Server *) server;
    int s_client;

    if(!listen(s->Socket, s->max_clients))
    {
        printf("[+] Listening on port:[%d]\n", s->port);

        while(s->Activated)
        {
            s_client = accept(s->Socket, NULL, NULL);
            ManageClient(s_client, s);

            sleep(1);
        }
    }
    else
    {
        printf("[-] Start listening failed: %s\n", strerror(errno));
    }
    return 0;
}

/* Monitor */
// Loops throught the client list and removes dead clients
void *ClientListMonitor(void *server)
{
    if(server == NULL) return 0;

    Server *s = (Server *) server;

    printf("[+] Started client monitor\n");

    while(s->Activated)
    {
        Lock_Client_Count(s);
        for(int i = 0;i < s->clients_connected;i++)
        {
            if(s->CLIENT[i]->Active == 0)
            {
                RemoveClient(s, i);
                break;
            }
        }
        Unlock_Client_Count(s);
        usleep(200000);
    }
    return 0;
}

/* Management  */
void ManageClient(int socket, Server *s)
{
    if(socket == -1 || s == NULL) return;

    Client *c;
    unsigned char token;
    int id;
    int err;
    
    // Receive initial message from client
    if(ReceiveInitialHeader(socket, &token, &id) == 0)
    {
        printf("[-] Could not receive initial header from Client with socket: %u\n", socket);
        return;
    }

    // If client is regular
    if(token == NORMAL_MODE)
    {
        // If Serer is full send SERVER_FULL token to client
        if(s->clients_connected >= s->max_clients){
            SendInitialHandshake(socket, SERVER_FULL, id);
            return;
        }

        // Check if the client is already in the database
        if(Client_In_Database(s, id) == 1)
        {
            // Create Client
            c = CreateClient(s, socket, id);
            
            // If internal error occured
            if(c == NULL)
            {
                SendInitialHandshake(socket, ABORD, id);
                return;
            }

            // Create client directory if it does not exists
            Create_Client_Directory(s, c->cloud_directory);

        }
        else
        {
            // Create new client with new id and directory
            c = CreateClient(s, socket, 0);

            // If internal error occured
            if(c == NULL)
            {
                SendInitialHandshake(socket, ABORD, id);
                return;
            }

            // Add new client to database
            if(Add_Client_To_Database(s, c->id, c->cloud_directory) == 0)
            {
                SendInitialHandshake(socket, ABORD, id);
                free(c);
                return;
            }
        }

        // Start client thread
        c->Active = 1;
        err = pthread_create(&c->h_client, NULL, &ClientRoutine, c);

        if(err != 0)
        {
            SendInitialHandshake(socket, ABORD, id);
            printf("[-] Could not start client thread: %s\n", strerror(err));
            free(c);
            return;
        }

        // Add client to server list
        Lock_Client_Count(s);
        s->CLIENT[s->clients_connected] = c;
        s->clients_connected ++;
        printf("[!] New client with id: %u\n", c->id);
        Unlock_Client_Count(s);

        // Send ok and id & check for fail
        if(SendInitialHandshake(c->socket, ALL_OK, c->id) == 0)
        {
            c->Active = 0;
        }
    }
    else if(token == ABORD)
    {
        return;
    }
}

// Creates a client
Client *CreateClient(Server *s, int socket, unsigned int id)
{
    if(s == NULL)
    {
        printf("[-] Server *s was null when trying to add client\n");
        return NULL;
    }
    if(socket == -1)
    {
        printf("[-] Tried to add INVALID_SOCKET Object or Server *s was NULL\n");
        return NULL;
    }

    struct sockaddr_in client_addr;
    Client *client = (Client *) malloc(sizeof(Client));
    if(client == NULL)
    {
        printf("[-] Could not allocate Memory for Client in CreateClient() %s\n", strerror(errno));
        return NULL;
    }

    socklen_t AddressLength = sizeof(struct sockaddr_in);
    getpeername(socket, (struct sockaddr *) &client_addr, &AddressLength);
    
    client->socket = socket;
    client->addr = client_addr;
    client->port = ntohs(client_addr.sin_port);
    client->ip = inet_ntoa(client_addr.sin_addr);

    // Client is added to server list and activatet in manage client
    client->Active = 0;

    // If client needs new unique id and directory
    client->cloud_directory = (char *) malloc((NAME_MAX + 1) * sizeof(char));
    if(client->cloud_directory == NULL) 
    {
        free(client);
        return NULL;
    }

    if(id == 0)
    {
        while(1){
            client->id = rand();
            if(!Client_In_Database(s, id)) break;
        }
        sprintf(client->cloud_directory, "%d/", client->id);
    }else
    {
        client->id = id;
        sprintf(client->cloud_directory, "%d/", client->id);
    }

    client->server_cloud_directory = (char *) malloc((strlen(s->config->cloud_directory) + 1) * sizeof(char));
    if(client->server_cloud_directory == NULL)
    {
        free(client);
        return NULL;
    }

    strncpy(client->server_cloud_directory, s->config->cloud_directory, (strlen(s->config->cloud_directory) + 1) * sizeof(char));

    return client;
}

/* Disposal */
// Removes a client by its index
void RemoveClient(Server *s, int index)
{
    if(s == NULL) return;
    if(s->Client_Lock == 0) return;
    if(index >= s->max_clients || index >= s->clients_connected) return;
    
    long int id;
    Client *c = s->CLIENT[index];
    int i = index;

    if(c->Active) c->Active = 0;

    // TODO Terminate Threads

    close(c->socket);

    id = c->id;

    free(c);

    for(;i < s->clients_connected - 1;i++)
    {
        s->CLIENT[i] = s->CLIENT[i + 1];
    }
    s->clients_connected --;
    printf("[!] Removed client with id: %ld\n", id);  
}
// Removes all clients by calling RemoveClient() until client list is empty
void RemoveAllClients(Server *s)
{
    Lock_Client_Count(s);
    while(s->clients_connected > 0)
    {
        RemoveClient(s, s->clients_connected - 1);
    }
    printf("[!] Removed all clients\n");
}
// Sets a client to inactiv
void ReportDisconnect(Client *client)
{
    if(client == NULL) return;
    client->Active = 0;
}

/* Thread locks */
// Prevent Client monitor and add client to change client list at the same time*/
int Lock_Client_Count(Server *s)
{
    while(!(s->Client_Lock == 0)){}
    s->Client_Lock = 1;
    return 1;
}
int Unlock_Client_Count(Server *s)
{
    while(!(s->Client_Lock == 1)){}
    s->Client_Lock = 0;
    return 1;
}