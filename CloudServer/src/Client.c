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
            case SEND_FILE:
                Receive_File(c, (char *) Buffer);
                break;

            case PULL_FILE:
                Send_File(c);
                break;

            case LIST_FILE:
                List_File(c);
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
    printf("Start send bytes\n");
    if(SendBytes(c, NULL, 0, SEND_FILE) == 0)
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
void Send_File(Client *c)
{
    if(c == NULL) return;
    if(!c->Active) return;


    
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
