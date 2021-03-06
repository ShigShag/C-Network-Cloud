#include "../inc/ClientBackend.h"
#include "../inc/Communication.h"
#include "../inc/ClientFrontend.h"
#include "../inc/Misc.h"
#include "../inc/Identity.h"

Client *Create_Client(char *config_file_path)
{
    Client *c = malloc(sizeof(Client));
    if(c == NULL)
    {
        printf("[-] Failed to Allocate Sapce for Client: %s\n", strerror(errno));
        return NULL;
    }

    c->config = Get_Config(config_file_path);
    if(c->config == NULL)
    {
        free(c);
        return NULL;
    }

    memset(&c->addr, 0, sizeof(c->addr));
    c->addr.sin_family = AF_INET;
    c->addr.sin_port = htons(c->config->port);
    c->addr.sin_addr.s_addr = inet_addr(c->config->ip);

    c->is_connected = 0;
    c->Active = 1;

    return c;
} 

int Connect_To_Server(Client *c)
{
    c->socket = socket(AF_INET, SOCK_STREAM, 0);

    int counter = 0;
    struct timeval tv;
    int err;

    if(c->config->receive_timeout > 0)
    {
        tv.tv_sec = c->config->receive_timeout;
        tv.tv_usec = 0;
        err = setsockopt(c->socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
        if(err != 0)
        {
            printf("[-] Could not set receive timeout: %s\n", strerror(errno));
        }else
        {
            printf("[+] Receive timeout set to %d seconds\n", c->config->receive_timeout);
        }
    }

    // Connect to server
    while(connect(c->socket, (struct sockaddr *)&c->addr, sizeof(c->addr)) == SOCKET_ERROR)
    {
        fflush(stdout);
        printf("Connecting to server, attempt: %d\r", counter++);
        sleep(1);
    }
    printf("\n");
    // Initial handshake
    if(Initial_Handshake(c) == 0) return 0;

    c->is_connected = 1;
    return 1;
}
int Initial_Handshake(Client *c)
{
    if(c == NULL) return 0;

    unsigned long id;
    unsigned char token;
    char *pw;
    unsigned long id_recv;
    unsigned int pw_count;

    if((id = Get_Client_Id(c)) == 0)
    {
        printf("[-] Id could not be read from: %s\n", c->config->identity_path);
        Create_Client_Id_File(c);
    }

    if(SendInitialHeader(c, NORMAL_MODE, id) == 0)
    {
        printf("[-] Could not send initial header to server\n");
        return 0;
    }

    token = ReceiveBytes(c, NULL, NULL);

    switch (token)
    {
    case SERVER_FULL:
            printf("[-] Server is full\n");
            return 0;

    case PASSWORD_REQUEST:
        printf("Enter password: ");
        pw = RetrievePassword(MAX_PASSWORD_SIZE, &pw_count);
        if(SendPassword(c, (int8_t *) pw, pw_count, PASSWORD_REQUEST) == 0)
        {
            printf("[-] Could not send password to server\n");
            return 0;
        }
        FreePassword(pw, &pw_count);
        printf("\n");
        break;
    
    case PASSWORD_NEW_REQUEST:
        printf("Enter new password: ");
        pw = RetrievePassword(MAX_PASSWORD_SIZE, &pw_count);
        if(SendPassword(c, (int8_t *)pw, pw_count, PASSWORD_NEW_REQUEST) == 0)
        {
            printf("[-] Could not send new password to server\n");
            return 0;
        }
        FreePassword(pw, &pw_count);
        printf("\n");
        break;

    default:
        printf("[-] Received unidentified token from server\n");
        return 0;
    }

    if(ReceiveInitialHandshake(c, &token, &id_recv) == 0)
    {
        printf("[-] Could not receive handshake from server\n");
        return 0;
    }

    switch (token)
    {
    case PASSWORD_DECLINED:
        printf("[-] Wrong password\n");
        return 0;
    
    case ABORD:
        printf("[-] Internal error in server occured\n");
        return 0;

    case ALL_OK:
        printf("[+] Server handshake ok\n");
        break;

    default:
        printf("[!] Received unidentified token from server %d\n", token);
        return 0;
    }

    Set_Client_Id(c, id_recv);
    c->id = id_recv;
    return 1;
}
void *Main_Routine_Back_End(void *arg)
{
    if(arg == NULL) return 0;

    Interface_arg *a = (Interface_arg *) arg;

    Client *c = a->client;
    Interface *i = a->interface;
    int token;

    while(1)
    {
        i->back_end_finished = 0;
        while(Connect_To_Server(c) != 1){sleep(5);}

        printf("Connected to [%s]:[%d]\n", c->config->ip, c->config->port);
        i->back_end_finished = 1;
        while(c->is_connected == 1 && c->Active)
        {
            if(i->front_end_finished)
            {
                /* Check if input was given */
                if(i->number_of_arguments < 1)
                {
                    Error_Interface(i, "Not engouh arguments");
                    i->front_end_finished = 0;
                    i->back_end_finished = 1;
                    continue;
                }
                
                token = Translate_Input(i->args[0]);

                switch (token)
                {
                case INTERNAL_EXIT:
                    c->Active = 0;
                    break;

                case INVALID_INPUT:
                    Error_Interface(i, "Wrong input");
                    break;

                case PUSH_FILE:
                    Push_File(c, i);
                    break;

                case PULL_FILE:
                    Pull_File(c, i);
                    break;

                case LIST_FILE:
                    List_File(c, i);
                    break;

                case DELETE_FILE:
                    Delete_File(c, i);
                    break;

                case FILE_CAT:
                    Cat_File(c, i);
                    break;

                case CLEAR:
                    Clear_Terminal();
                    break;

                case HELP:
                    Help(i);
                    break;
                
                default:
                    break;
                }
             
                i->front_end_finished = 0;
                i->back_end_finished = 1;
            }
            usleep(1000 * 100);
        }
        if(c->Active == 0) break;
    }
    Delete_Client(c);
    return 0;
}
int Translate_Input(char *input)
{
    if(input == NULL) return 0;
 
    /* Token are defined in Communication.h */
    if(!strcmp(input, "push")){
        return PUSH_FILE;
    }
    if(!strcmp(input, "exit")){
        return INTERNAL_EXIT;
    }
    if(!strcmp(input, "list") || !strcmp(input, "ls")){
        return LIST_FILE;
    }
    if(!strcmp(input, "pull")){
        return PULL_FILE;
    }
    if(!strcmp(input, "pushfast")){
        return PUSH_FILE_FAST;
    }
    if(!strcmp(input, "delete") || !strcmp(input, "rm")){
        return DELETE_FILE;
    }
    if(!strcmp(input, "cat")){
        return FILE_CAT;
    }
    if(!strcmp(input, "help")){
        return HELP;
    }
    if(!strcmp(input, "clear") || !strcmp(input, "cls")){
        return CLEAR;
    }
    else{
        return INVALID_INPUT;
    }
}

void Push_File(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    if(c->Active == 0) return;

    char *f_path;
    char *f_name;
    int token_recv;
    
    if(i->number_of_arguments < 2)
    {
        Error_Interface(i, "Not enough arguments for push");
        return;
    }

    f_path = i->args[1];

    //printf("path: %s\n", f_path);
    int fd = open(f_path, O_RDONLY);
    if(fd  == -1)
    {
        printf("Could not open file %s: %s\n", f_path, strerror(errno));
        return;
    }
    
    f_name = basename(f_path);
    printf("basename: %s\n", f_name);

    if(SendBytes(c, (uint8_t *) f_name, strlen(f_name) + 1, PUSH_FILE) == 0)
    {
        Error_Interface(i, "Could not send Token for push file");
        close(fd);
        return;
    }

    if((token_recv = ReceiveBytes(c, NULL, NULL)) != PUSH_FILE)
    {
        switch (token_recv)
        {
        case ERROR_TOKEN:
            Error_Interface(i, "Server returned an error");
            break;

        case FILE_ALREADY_EXISTS:
            Error_Interface(i, "File already exists on server");
            break;
        
        default:
            Error_Interface(i, "Server returned undefined token");
            break;
        }
        close(fd);
        return;
    }

    SendFile_t(c, fd);
    close(fd);

    Output_Interface(i, "Send File was successfull\n");
}
/* File pulling */
void Pull_File(Client *c, Interface *i)
{
    if(c == NULL || i == NULL) return;
    if(c->Active == 0) return;

    if(i->number_of_arguments < 3)
    {
        Error_Interface(i, "Not enough arguments for pull");
        return;
    }

    FILE *fp;

    int token;
    uint64_t bytes_received;

    char *file_name;
    char *destination_path;

    file_name = basename(i->args[1]);
    destination_path = i->args[2];

    if(SendBytes(c, (uint8_t *) file_name, strlen(file_name) + 1, PULL_FILE) == 0)
    {
        Error_Interface(i, "Could not send Token for pull file");
        return;
    }

    if((token = ReceiveBytes(c, NULL, NULL)) != PULL_FILE)
    {
        switch (token)
        {
        case ERROR_TOKEN:
            Error_Interface(i, "Server returned an error");
            break;

        case FILE_DOES_NOT_EXIST:
            Error_Interface(i, "File does not exist on server");
            break;

        default:
            Error_Interface(i, "Server retruned undefined token");
            break;
        } 
        return;     
    }

    fp = fopen(destination_path, "wb");
    if(fp == NULL){
        Error_Interface(i, "Could not open destination file");
        return;
    }

    bytes_received = ReceiveFile(c, fp);

    printf("Received: %ld bytes\n", bytes_received);

    fclose(fp);
}
void List_File(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    if(c->Active == 0) return;

    if(SendBytes(c, NULL, 0, LIST_FILE) == 0)
    {
        Error_Interface(i, "Could not send Token for file list");
        return;
    }
    
    unsigned char *buffer;
    uint64_t bytes_received;
    int token;

    token = ReceiveBytes(c, &buffer, &bytes_received);
    if(token == 0)
    {
        Error_Interface(i, "Receive bytes failed while trying to receive list\n");
        if(buffer) free(buffer);
        return;
    }
    else if(token == ERROR_TOKEN)
    {
        Error_Interface(i, "Server did not answer list file request\n");
        if(buffer) free(buffer);
        return;
    }

    Output_Interface(i, (char *) buffer);
    if(buffer) free(buffer);
    return;
}
void Delete_File(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    if(c->Active == 0) return;  

    if(i->number_of_arguments < 2)
    {
        Error_Interface(i, "Not enough arguments for delete");
        return;
    }

    int token;
    char *file_name = basename(i->args[1]);

    if(SendBytes(c, (uint8_t *) file_name, strlen(file_name) + 1, DELETE_FILE) == 0)
    {
        Error_Interface(i, "Could not send token for DELETE_FILE");
        return; 
    }

    if((token = ReceiveBytes(c, NULL, NULL)) != DELETE_FILE)
    {
        switch(token)
        {
        case FILE_DOES_NOT_EXIST:
            Error_Interface(i, "File does not exist on server");
            break;

        case ERROR_TOKEN:
            Error_Interface(i, "Error on server");
            break;
        
        default:
            Error_Interface(i, "Did not receive DELETE_FILE token from server");
            break;
        }
        return;
    }

    Output_Interface(i, "[+] File was deleted");
}
void Cat_File(Client *c, Interface *i)
{
    if(c == NULL || i == NULL) return;
    if(c->Active == 0) return;

    char *f_name;
    int token;

    if(i->number_of_arguments < 2)
    {
        Error_Interface(i, "Not enough arguments for cat");
        return;
    }

    f_name = i->args[1];

    if(SendBytes(c, (uint8_t *) f_name, strlen(f_name) + 1, FILE_CAT) == 0)
    {
        Error_Interface(i, "Could not send Token for cat file");
        return;
    }

    if((token = ReceiveBytes(c, NULL, NULL)) != FILE_CAT)
    {
        switch (token)
        {
        case ERROR_TOKEN:
            Error_Interface(i, "Server returned an error");
            break;

        case FILE_DOES_NOT_EXIST:
            Error_Interface(i, "File does not exist on server");
            break;

        default:
            Error_Interface(i, "Server retruned undefined token");
            break;
        } 
        return;     
    }

    ReceiveFile(c, stdout);
}
void Help(Interface *i)
{
    char *help = "Commands:\n"
    "exit                                        -> exits the application\n"
    "list                                        -> lists all the files on the server\n"
    "pull   [file name] [destination file name]  -> pull a file from the server\n"
    "push   [file name]                          -> push a local file onto the server\n"
    "cat    [file name]                          -> shows the content of a file from the server\n"
    "delete [file name]                          -> delete a file from the server\n"
    "cls/clear                                   -> clears the terminal\n"
    "help                                        -> displays this text\n";
    Output_Interface(i, help);
}
void Clear_Terminal()
{
    system("clear");
}
void ReportDisconnect(Client *c)
{
    if(c == NULL) return;

    c->is_connected = 0;
}
void Delete_Client(Client *c)
{
    if(c == NULL) return;

    c->Active = 0;
    
    Delete_Config(c->config);
    close(c->socket);
    free(c);
    printf("[+] Client was deleted\n");
}