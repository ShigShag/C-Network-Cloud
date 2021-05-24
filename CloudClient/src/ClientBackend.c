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

client_t *Create_Transmission_Client(char *ip, int port, int p_id)
{
    if(ip == NULL || port <= 0) return NULL;

    client_t *c = (client_t *) malloc(sizeof(client_t));
    if(c == NULL)
    {
        printf("[-] Failed to Allocate Sapce for transmission Client: %s\n", strerror(errno));
        return NULL;
    }

    memset(&c->addr, 0, sizeof(c->addr));
    c->addr.sin_family = AF_INET;
    c->addr.sin_port = htons(port);
    c->addr.sin_addr.s_addr = inet_addr(ip);
    
    c->p_id = p_id;
    c->is_connected = 0;
    c->thread = 0;
    return c;
}
int Connect_Transmission_Client(client_t *c)
{
    c->socket = socket(AF_INET, SOCK_STREAM, 0);

    // Connect to server
    while(connect(c->socket, (struct sockaddr *) &c->addr, sizeof(c->addr)) == SOCKET_ERROR)
    {
        usleep(100000);
    }
    if(Initial_Handshake_t(c) == 0) return 0;

    c->is_connected = 1;
    return 1;
}
int Initial_Handshake_t(client_t *c)
{
    if(c == NULL) return 0;

    unsigned char token;

    if(SendTransmissionClientHeader(c, DOWNLOAD_MODE) == 0)
    {
        printf("[-] Could not send initial header to server from transmission client\n");
        return 0;
    }
    if(ReceiveInitialHandshake_t(c, &token) == 0)
    {
        printf("[-] Could not receive handshake from server in transmission client\n");
        return 0;       
    }

    switch(token)
    {
    case ABORD:
        printf("[-] Received abord from server in transmission client\n");
        return 0;

    case ALL_OK:
        return 1;
    
    default:
        printf("[-] Received unidentified token from server in transmission client: %d\n", token);
        return 0;
    }
}

void *Send_Packet_t(void *arg)
{
    if(arg == NULL) return NULL;

    unsigned long bytes_send = SendFile_f((SEND_ARG *) arg);
    printf("Send %ld bytes\n", bytes_send);

    free(arg);
}
void Delete_Transmission_Client(client_t *c)
{
    if(c == NULL) return;

    close(c->socket);
    free(c);
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
    if(!strcmp(input, "delete") || !strcmp(input, "rm")){
        return DELETE_FILE;
    }
    else{
        return INVALID_INPUT;
    }
}

/* File transmition */
void Push_File_f(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    if(c->Active == 0) return;

    char *f_path;
    char *f_name;
    int token_recv;
    client_t *t_arr[DYNAMIC_CLIENT_TRANSMISSION_COUNT];
    struct stat f_stat;

    /* sending packets */
    long block_size ;
    long remainder;

    if(i->number_of_arguments < 2)
    {
        Error_Interface(i, "Not enough arguments for push");
        return;
    }

    f_path = i->args[1];

    int fd = open(f_path, O_RDONLY);
    if(fd  == -1)
    {
        printf("Could not open file: %s\n", strerror(errno));
        return;
    }

    if(fstat(fd, &f_stat) <  0)
    {
        printf("[-] fstat failed: %s\n", strerror(errno));
        return;
    }
    
    f_name = basename(f_path);

    for(int i = 0;i < DYNAMIC_CLIENT_TRANSMISSION_COUNT;i++)
    {
        client_t *temp = Create_Transmission_Client(c->config->ip, c->config->port, c->id);
        if(temp == NULL)
        {
            for(int n = 0;n < i;n++)
            {
                Delete_Transmission_Client(t_arr[n]);
            }
            printf("Could not allocate space for transmission client\n");
            close(fd);
            return;
        }
        t_arr[i] = temp;
    }

    if(SendBytes(c, (uint8_t *) f_name, strlen(f_name) + 1, PUSH_FILE_FAST) == 0)
    {
        Error_Interface(i, "Could not send Token for push file fast");
        close(fd);
        return;
    }

    if((token_recv = ReceiveBytes(c, NULL, NULL)) != PUSH_FILE_FAST)
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

    block_size = f_stat.st_size / DYNAMIC_CLIENT_TRANSMISSION_COUNT;
    remainder = f_stat.st_size % DYNAMIC_CLIENT_TRANSMISSION_COUNT;

    for(int i = 0;i < DYNAMIC_CLIENT_TRANSMISSION_COUNT;i++)
    {
        if(Connect_Transmission_Client(t_arr[i]) == 0)
        {
            // TODO
        } 
    }

    for(int i = 0;i < DYNAMIC_CLIENT_TRANSMISSION_COUNT;i++)
    {
        SEND_ARG *arg = (SEND_ARG *) malloc(sizeof(SEND_ARG));
        if(arg == NULL) continue;

        off_t offset = lseek(fd, block_size * i, SEEK_SET);

        arg->count = block_size;
        arg->in = fd;
        arg->out = t_arr[i]->socket;
        arg->offset = offset;

        int err = pthread_create(&t_arr[i]->thread, NULL, Send_Packet_t, arg);
        if(err != 0)
        {
            printf("[-] Could not create thread: %s\n", strerror(errno));
        }
    }

    // Remainder weiter machen

    close(fd);
}

void Push_File(Client *c, Interface *i)
{
    if(i == NULL || c == NULL) return;
    if(c->Active == 0) return;

    char *f_path;
    char *f_name;
    unsigned long bytes_send;
    int token_recv;
    clock_t begin, end;
    double total_time;
    ProgressBar *pb;
    
    if(i->number_of_arguments < 2)
    {
        Error_Interface(i, "Not enough arguments for push");
        return;
    }

    f_path = i->args[1];

    printf("path: %s\n", f_path);
    int fd = open(f_path, O_RDONLY);
    if(fd  == -1)
    {
        printf("Could not open file: %s\n", strerror(errno));
        return;
    }
    
    pb = Create_Progress_Bar(1, NULL, '#');
    if(pb == NULL)
    {
        printf("Could not create Progress bar: %s\n", strerror(errno));
        return;
    }

    f_name = basename(f_path);
    printf("basename: %s\n", f_name);

    if(SendBytes(c, (uint8_t *) f_name, strlen(f_name) + 1, PUSH_FILE) == 0)
    {
        Error_Interface(i, "Could not send Token for push file");
        close(fd);
        Delete_Progress_Bar(pb);
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
        Delete_Progress_Bar(pb);
        return;
    }

    begin = clock();
    bytes_send = SendFile_t(c, fd);
    end = clock();
    total_time = (double)(end - begin) / CLOCKS_PER_SEC;
    close(fd);
    Delete_Progress_Bar(pb);

    printf("%ld Bytes were send in %f seconds\n", bytes_send, total_time);

    Output_Interface(i, "Send File was successfull");
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

    clock_t begin, end;
    double total_time;

    file_name = i->args[1];
    destination_path = i->args[2];

    fp = fopen(destination_path, "wb");
    if(fp == NULL)
    {
        Error_Interface(i, "Could not open destination file");
        return;
    }

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

    begin = clock();
    bytes_received = ReceiveFile(c, fp);
    end = clock();

    total_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Received: %ld bytes in %f seconds\n", bytes_received, total_time);

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
    char *file_name = i->args[1];

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