#include "../inc/ClientManagment.h"
#include "../inc/Communication.h"
#include "../inc/Misc.h"
#include "../inc/Crypto.h"
#include "../inc/Logging.h"

/* Listening */
// Listens for new client and passes them to the manage function
void *ClientListeningThread(void *server)
{
    if(server == NULL) return 0;

    srand(time(NULL));

    Server *s = (Server *) server;
    int s_client;

    if(listen(s->Socket, s->max_clients) == 0)
    {
        WriteLog(s->log, 1, LOG_SUCCESS, "Listening on port:[%d]", s->port);

        while(s->Activated)
        {
            s_client = accept(s->Socket, NULL, NULL);
            ManageClient(s_client, s);
            
            sleep(1);
        }
    }
    else
    {
        WriteLog(s->log, 1, LOG_FAIL, "Start listening failed: [%s]", strerror(errno));
    }
    return 0;
}

/* Monitor */
// Loops throught the client list and removes dead clients
void *ClientListMonitor(void *server)
{
    if(server == NULL) return 0;

    Server *s = (Server *) server;

    WriteLog(s->log, 1, LOG_SUCCESS, "Started client monitor", strerror(errno));


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
    unsigned long id;
    int err;
    
    // Receive initial message from client
    if(ReceiveInitialHeader(socket, &token, &id) == 0)
    {
        WriteLog(s->log, 1, LOG_FAIL, "Could not receive initial header from Client with socket: %u", socket);
        return;
    }

    printclientip(socket);

    // If client is regular
    if(token == NORMAL_MODE)
    {
        // If Serer is full send SERVER_FULL token to client
        if(s->clients_connected >= s->max_clients){
            SendServerFull(socket);
            return;
        }

        // Check if the client is already in the database
        if(Client_In_Database(s, id) == 1)
        {
            // Send password request
            if(SendPasswordRequest(socket) == 0) return;

            int same;
            int8_t *pw;
            uint8_t token;

            token = ReceivePassword(socket, &pw);
            if(token != PASSWORD_REQUEST)
            {
                switch (token)
                {
                    case 0: return;
                    default: 
                        SendInitialHandshake(socket, PASSWORD_DECLINED, id); 
                        break;
                }
            }

            // Check if password was correct
            same = Check_Client_Password(s, id, (char *) pw);
            
            // Delete password from memory
            free_memset(pw, strlen((char *) pw));

            if(same == 0)
            {
                SendInitialHandshake(socket, PASSWORD_DECLINED, id);
                return;
            }

            // Create Client
            c = CreateClient(s, socket, id);
            
            // If internal error occured
            if(c == NULL)
            {
                SendInitialHandshake(socket, ABORD, id);
                return;
            }

            // Create client directory if it does not exists -> The function will create a new directory if it doesnt already exist -> just to make sure
            Create_Client_Directory(s, c->directory);
        }
        else
        {
            if(SendNewPasswordRequest(socket) == 0) return;

            int8_t *pw;
            uint8_t token;

            token = ReceivePassword(socket, &pw);
            if(token != PASSWORD_NEW_REQUEST)
            {
                switch (token)
                {
                    case 0: return;
                    default: 
                        SendInitialHandshake(socket, PASSWORD_DECLINED, id); 
                        break;
                }            
            }

            // Create new client with new id and directory
            c = CreateClient(s, socket, 0);

            // If internal error occured
            if(c == NULL)
            {
                SendInitialHandshake(socket, ABORD, id);
                free_memset(pw, strlen((char *) pw));
                return;
            }

            if(Add_Client_credentials(s, c->id, (char *) pw) == 0)
            {
                SendInitialHandshake(socket, PASSWORD_DECLINED, c->id);
                free(c);
                free_memset(pw, strlen((char *) pw));
            }

            // Delete password from memory
            free_memset(pw, strlen((char *) pw));

            // Add new client to database and create a new cloud directory
            if(Add_Client_To_Database(s, c->id, c->directory) == 0)
            {
                SendInitialHandshake(socket, ABORD, c->id);
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
            WriteLog(s->log, 1, LOG_FAIL, "Could not start client thread: %s", strerror(err));
            free(c);
            return;
        }

        // Add client to server list
        Lock_Client_Count(s);
        s->CLIENT[s->clients_connected] = c;
        s->clients_connected ++;
        Unlock_Client_Count(s);
        WriteLog(s->log, 1, LOG_NOTICE, "New client with id: [%lu]:[%s]:[%d]", c->id, c->ip, c->port);

        // Send ok and id & check for fail
        if(SendInitialHandshake(c->socket, ALL_OK, c->id) == 0)
        {
            c->Active = 0;
        }
    }
    else if(token == DOWNLOAD_MODE)
    {
        Lock_Client_Count(s);
        for(int i = 0;i < s->clients_connected;i++)
        {
            if(s->CLIENT[i]->id == id  && s->CLIENT[i]->transmission_client_allowed == 1)
            {
                if(s->CLIENT[i]->transmission_client_count < DYNAMIC_CLIENT_TRANSMISSION_COUNT)
                {
                    s->CLIENT[i]->transmission_client_array[s->CLIENT[i]->transmission_client_count++] = socket;
                }
            }
        }
        Unlock_Client_Count(s);
    }
}

// Creates a client
Client *CreateClient(Server *s, int socket, unsigned long id)
{
    if(s == NULL)
    {
        return NULL;
    }
    if(socket == -1)
    {
        return NULL;
    }

    struct sockaddr_in client_addr;
    Client *client = (Client *) malloc(sizeof(Client));
    if(client == NULL)
    {
        WriteLog(s->log, 1, LOG_FAIL, "Could not allocate Memory for Client in CreateClient() %s", strerror(errno));
        return NULL;
    }

    socklen_t AddressLength = sizeof(struct sockaddr_in);
    getpeername(socket, (struct sockaddr *) &client_addr, &AddressLength);
    
    client->socket = socket;
    client->addr = client_addr;
    client->port = ntohs(client_addr.sin_port);
    client->ip = inet_ntoa(client_addr.sin_addr);

    client->transmission_client_allowed = 0;
    client->transmission_client_count = 0;
    client->transmission_client_array = (int *) calloc(DYNAMIC_CLIENT_TRANSMISSION_COUNT, sizeof(int));
    if(client->transmission_client_array == NULL)
    {
        free(client);
        return NULL;
    }

    // Client is added to server list and activatet in manage client
    client->Active = 0;

    // If client needs new unique id and directory
    client->cloud_directory = (char *) malloc((NAME_MAX + 1) * sizeof(char));
    if(client->cloud_directory == NULL) 
    {
        free(client);
        return NULL;
    }

    // Check if id is given if not create a new one
    if(id == 0)
    {
        unsigned long r;
        // Generate Client ID
        while(1){
            get_random_unsigned_long(&r);
            if(Client_In_Database(s, r) == 0) break;
        }
        client->id = r;

        // Generate Client Cloud directory name
        while(1){
            get_random_unsigned_long(&r);
            if(Directory_In_Database(s, r) == 0) break;
        }
        client->directory = r;

        get_random_unsigned_long(&r);
    }else
    {
        client->id = id;
        client->directory = Get_Client_Directory(s, client->id);
    }
    snprintf(client->cloud_directory, (NAME_MAX + 1) * sizeof(char), "%lu/", client->directory);

    client->server_cloud_directory = (char *) malloc((strlen(s->config->cloud_directory) + 1) * sizeof(char));
    if(client->server_cloud_directory == NULL)
    {
        free(client);
        return NULL;
    }

    // This line makes sense
    strncpy(client->server_cloud_directory, s->config->cloud_directory, (strlen(s->config->cloud_directory) + 1) * sizeof(char));

    client->completed_cloud_directory = (char *) malloc((PATH_MAX + 1) * sizeof(char));
    if(client->server_cloud_directory == NULL)
    {
        free(client);
        return NULL;
    }
    client->completed_cloud_directory = append_malloc(client->server_cloud_directory, client->cloud_directory);
    return client;
}

/* Disposal */
// Removes a client by its index
void RemoveClient(Server *s, int index)
{
    if(s == NULL) return;
    if(s->Client_Lock == 0) return;
    if(index >= s->max_clients || index >= s->clients_connected) return;
    
    unsigned long id;
    Client *c = s->CLIENT[index];
    int i = index;

    if(c->Active) c->Active = 0;

    // TODO Terminate Threads
    free(c->transmission_client_array);
    close(c->socket);
    free(c->cloud_directory);
    free(c->server_cloud_directory);
    free(c->completed_cloud_directory);

    id = c->id;

    free(c);

    for(;i < s->clients_connected - 1;i++)
    {
        s->CLIENT[i] = s->CLIENT[i + 1];
    }
    s->clients_connected --;
    WriteLog(s->log, 1, LOG_NOTICE, "Removed client with id: [%lu]", id);
}
// Removes all clients by calling RemoveClient() until client list is empty
void RemoveAllClients(Server *s)
{
    Lock_Client_Count(s);
    while(s->clients_connected > 0)
    {
        RemoveClient(s, s->clients_connected - 1);
    }
    WriteLog(s->log, 1, LOG_NOTICE, "Removed all clients");
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