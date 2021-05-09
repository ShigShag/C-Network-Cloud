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

    c->aes_key = NULL;
    c->is_connected = 0;
    c->Active = 1;

    return c;
} 
int Connect_To_Server(Client *c)
{
    c->socket = socket(AF_INET, SOCK_STREAM, 0);

    int counter = 0;

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

    int id = Get_Client_Id(c);
    unsigned char token;
    int id_recv;

    // If internal error occured
    if(id < 0)
    {
        printf("[-] Id could not be read from: %s\n", c->config->identity_path);
        SendInitialHeader(c, ABORD, 0);
        return 0;
    }

    if(SendInitialHeader(c, NORMAL_MODE, id) == 0)
    {
        printf("[-] Could not send initial header to server\n");
        return 0;
    }

    if(ReceiveInitialHandshake(c, &token, &id_recv) == 0)
    {
        printf("[-] Could not receive handshake from server\n");
        return 0;
    }

    switch (token)
    {
    case SERVER_FULL:
        printf("[-] Server is full\n");
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
                    Send_File(c, i);
                    break;

                case PULL_FILE:
                    Pull_File(c, i);
                    break;

                case LIST_FILE:
                    List_File(c, i);
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
    if(!strcmp(input, "list")){
        return LIST_FILE;
    }
    if(!strcmp(input, "pull")){
        return PULL_FILE;
    }
    else{
        return INVALID_INPUT;
    }
}

/* File transmition */
void Send_File(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    char *f_path;
    char *f_name;
    unsigned long bytes_send;
    int token_recv;
    clock_t begin, end;
    double total_time;

    if(i->number_of_arguments < 2)
    {
        Error_Interface(i, "Not enough arguments for push");
        return;
    }

    /*f_path = Get_File_Path_Via_Dialog();
    if(f_path == NULL)
    {
        Error_Interface(i, "Could not gather the full path of the file");
        return;
    }

    int fd = open(f_path, O_RDONLY);*/
    int fd = open(i->args[1], O_RDONLY);
    if(fd  == -1)
    {
        printf("Could not  open file: %s\n", strerror(errno));
        return;
    }

    f_name = basename(f_path);

    if(SendBytes(c, (uint8_t *) f_name, strlen(f_name) + 1, PUSH_FILE) == 0)
    {
        Error_Interface(i, "Could not send Token for file transmition");
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
        return;
    }

    begin = clock();
    bytes_send = SendFile_t(c, fd);
    end = clock();
    total_time = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("%ld Bytes were send in %f seconds\n", bytes_send, total_time);

    Output_Interface(i, "Send File was successfull");
}
/* File pulling */
void Pull_File(Client *c, Interface *i)
{
    if(c == NULL || i == NULL) return;


}
void List_File(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    if(c->Active == 0) return;

    uint32_t s;
    s = SendBytes(c, NULL, 0, LIST_FILE);
    printf("bytes send: %d\n", s);
    if(s == 0)
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
void ReportDisconnect(Client *c)
{
    if(c == NULL) return;

    c->is_connected = 0;
}
void Delete_Client(Client *c)
{
    if(c == NULL) return;

    c->Active = 0;
    
    free(c->config);
    close(c->socket);
    free(c);
    printf("[+] Client was deleted\n");
}