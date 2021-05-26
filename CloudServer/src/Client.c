#include <time.h>
#include <stdio.h>
#include "../inc/Client.h"
#include "../inc/Communication.h"
#include "../inc/AesUtility.h"
#include "../inc/Misc.h"

#define NULL_CHECK(val) if (val == NULL) continue;

void *ClientRoutine(void *client)
{
    if(client == NULL) return NULL;

    Client *c = (Client *) client;

    uint8_t *Buffer;
    uint64_t BytesReceived;
    int32_t Token;
    
    while(c->Active)
    {
        Token = ReceiveBytes(client, &Buffer, &BytesReceived);
        printf("Token Received: %d\n", Token);
        printf("Bytes Received: %ld\n", BytesReceived);
        if(Token == 0)
        {
            c->Active = 0;
            continue;
        }

        switch(Token)
        {
            case PUSH_FILE:
                Receive_File(c, (char *) Buffer);
                break;

            case PULL_FILE:
                Send_File(c, (char *) Buffer);
                break;

            case PUSH_FILE_FAST:
                Receive_File_Fast(c, (char *) Buffer);

            case LIST_FILE:
                List_File(c);
                break;

            case DELETE_FILE:
                Delete_File(c, (char *) Buffer);
                break;
 
            default:
                continue;
        }
    }

    return 0;
}
void Receive_File(Client *c, char *f_name)
{
    if(c == NULL) return;
    if(!c->Active) return;

    FILE *fp;
    char *path = append_malloc(c->server_cloud_directory, c->cloud_directory);
    append_realloc(&path, f_name);
    unsigned long bytes_received;
    clock_t begin, end;
    double total_time;

    if(File_Exists(path))
    {
        free(path);
        SendBytes(c, NULL, 0, FILE_ALREADY_EXISTS);
        printf("File already exists\n");
        return;
    }

    fp = fopen(path, "wb");

    if(fp == NULL)
    {
        free(path);
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("Could not open file\n");
        return;
    }

    if(SendBytes(c, NULL, 0, PUSH_FILE) == 0)
    {
        free(path);
        printf("Send bytes failed\n");
        return;
    }

    begin = clock();
    bytes_received = ReceiveFile(c, fp);
    end = clock();
    total_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Received: %ld bytes in %f seconds\n", bytes_received, total_time);

    fclose(fp);
    free(path);
}
void Receive_File_Fast(Client *c, char *f_name)
{
    if(c == NULL) return;
    if(!c->Active) return;

    char *path;
    FILE *fp;

    path = append_malloc(c->server_cloud_directory, c->cloud_directory);
    append_realloc(&path, f_name);

    if(File_Exists(path))
    {
        free(path);
        SendBytes(c, NULL, 0, FILE_ALREADY_EXISTS);
        printf("File already exists\n");
        return;
    }

    fp = fopen(path, "wb+");

    if(fp == NULL)
    {
        free(path);
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("Could not open file\n");
        return;
    }

    c->transmission_client_allowed = 1;

    if(SendBytes(c, NULL, 0, PUSH_FILE_FAST) == 0)
    {
        free(path);
        printf("Send bytes failed\n");
        fclose(fp);
        return;
    }

    /* Wait for transmission clients to connect -> Total wait time 2 seconds */
    int count = 0; 
    while(c->transmission_client_count != DYNAMIC_CLIENT_TRANSMISSION_COUNT - 1 && count <= 2)
    {
        sleep(1);
        count ++;
    }
    if(count > 2 || c->transmission_client_count != DYNAMIC_CLIENT_TRANSMISSION_COUNT - 1)
    {
        free(path);
        Reset_Client_Transmission_Array(c);
        fclose(fp);
        return;
    }

    pthread_t thread_array[DYNAMIC_CLIENT_TRANSMISSION_COUNT];

    for(int i = 0;i < DYNAMIC_CLIENT_TRANSMISSION_COUNT;i++)
    {
        RECV_ARG *arg = (RECV_ARG *) malloc(sizeof(RECV_ARG));
        if(arg == NULL) continue;

        arg->in = c->transmission_client_array[i];
        arg->out = fp;

        int err = pthread_create(&thread_array[i], NULL, Receive_Packet_t, arg);
        if(err != 0)
        {
            printf("[-] Could not create thread: %s\n", strerror(errno));
        }
    }

    for(int i = 0;i < DYNAMIC_CLIENT_TRANSMISSION_COUNT;i++)
    {
        pthread_join(thread_array[i], NULL);
    }   
    
    fclose(fp);
    Reset_Client_Transmission_Array(c);
}
void Send_File(Client *c, char *f_name)
{
    if(c == NULL || f_name == NULL) return;
    if(!c->Active) return;

    int fd;
    char *path;
    
    double begin, end;
    float total_time;

    uint64_t bytes_send;
    
    path = append_malloc(c->server_cloud_directory, c->cloud_directory);
    if(path == NULL)
    {
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        return;
    }
    append_realloc(&path, f_name);

    fd = open(path, O_RDONLY);

    printf("path: %s\n", path);

    if(fd == -1)
    {
        free(path);
        SendBytes(c, NULL, 0, FILE_DOES_NOT_EXIST);
        printf("File does not exist\n");
        return;
    }

    if(SendBytes(c, NULL, 0, PULL_FILE) == 0)
    {
        free(path);
        printf("Send bytes failed\n");
        close(fd);
        return;
    }

    begin = clock();
    bytes_send = SendFile_t(c, fd);
    end = clock();
    total_time = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%ld bytes send in %f seconds\n", bytes_send, total_time);

    free(path);
    close(fd);
}
void List_File(Client *c)
{
    if(c == NULL) return;
    if(!c->Active) return;

    char *dir = (char *) malloc((NAME_MAX + 1) * sizeof(char));
    if(dir == NULL)
    {
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("[-] Could not allocate space for dir in List_File\n");
        return; 
    }

    dir = append_malloc(c->server_cloud_directory, c->cloud_directory);
    if(dir == NULL)
    {
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("[-] Could not allocate dir\n");
        return;
    }
    printf("dir: %s\n", dir);

    char *dir_list = Get_Directory_List(dir);
    if(dir_list == NULL)
    {
        free(dir);
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("[-] Could not get directory list\n");
        return;
    }

    if(SendBytes(c, (uint8_t *) dir_list, strlen(dir_list), LIST_FILE) == 0)
    {
        free(dir);
        free(dir_list);
        printf("Send bytes failed\n");
        return;
    }

    free(dir);
    free(dir_list);
    printf("List was send to client\n");
}

/* Deletes a file */
void Delete_File(Client *c, char *f_name)
{
    if(c == NULL) return;
    if(!c->Active) return;

    char *path;

    if(f_name == 0)
    {
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("File name was NULL\n");
        return;
    }

    path = append_malloc(c->server_cloud_directory, c->cloud_directory);
    if(path == NULL)
    {
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        printf("Could not allocate space for path in DELETE_FILE %s\n", strerror(errno));
        return;       
    }
    append_realloc(&path, f_name);

    if(remove(path) == -1)
    {
        SendBytes(c, NULL, 0, FILE_DOES_NOT_EXIST);
        printf("Could not delete file: %s -> %s\n", path, strerror(errno));
        free(path);
        return;
    }

    SendBytes(c, NULL, 0, DELETE_FILE);
    free(path);
}
void *Receive_Packet_t(void *arg)
{
    if(arg == NULL) return (void *) 1;

    unsigned long bytes_received = ReceiveFile_f(arg);
    printf("Bytes received: %ld bytes\n", bytes_received);

    free(arg);
    return NULL;
}
void Reset_Client_Transmission_Array(Client *c)
{
    if(c == NULL) return;

    c->transmission_client_allowed = 0;
    memset(c->transmission_client_array, 0, DYNAMIC_CLIENT_TRANSMISSION_COUNT * sizeof(int));
    c->transmission_client_count = 0;
}