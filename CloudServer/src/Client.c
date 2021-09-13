#include <time.h>
#include <stdio.h>
#include "../inc/Client.h"
#include "../inc/Communication.h"
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
        WriteLog(c->log, 1, LOG_NOTICE, "Token: %d", Token);
        WriteLog(c->log, 1, LOG_NOTICE, "Message Size in bytes: %lu", BytesReceived);

        if(Token == 0)
        {
            WriteLog(c->log, 1, LOG_WARNING, "Received Token 0 from client -> exiting...");
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

            case LIST_FILE:
                List_File(c);
                break;

            case DELETE_FILE:
                Delete_File(c, (char *) Buffer);
                break;

            case CAT_FILE:
                Cat_File(c, (char *) Buffer);
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

    WriteLog(c->log, 1, LOG_NOTICE, "Client wants to push file: [%s]", f_name);

    FILE *fp;
    char *path = append_malloc(c->complete_cloud_directory, f_name);
    unsigned long bytes_received;

    if(File_Exists(path))
    {
        free(path);
        SendBytes(c, NULL, 0, FILE_ALREADY_EXISTS);
        WriteLog(c->log, 1, LOG_FAIL, "File: [%s] already exists", f_name);
        return;
    }

    fp = fopen(path, "wb");

    if(fp == NULL)
    {
        free(path);
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        WriteLog(c->log, 1, LOG_FAIL, "File: [%s] could not be opened", f_name);
        return;
    }

    if(SendBytes(c, NULL, 0, PUSH_FILE) == 0)
    {
        free(path);
        WriteLog(c->log, 1, LOG_WARNING, "Could not send bytes to client");
        return;
    }

    bytes_received = ReceiveFile(c, fp);
    if(bytes_received == 0) WriteLog(c->log, 1, LOG_FAIL, "Could not receive file because of socket error - [%lu] bytes were received", bytes_received);
    else WriteLog(c->log, 1, LOG_SUCCESS, "File: [%s] was received - size: [%lu]", f_name, bytes_received);

    fclose(fp);
    free(path);
}
void Send_File(Client *c, char *f_name)
{
    if(c == NULL || f_name == NULL) return;
    if(!c->Active) return;

    WriteLog(c->log, 1, LOG_NOTICE, "Client wants to pull file: [%s]", f_name);

    int fd;
    char *path;
    
    uint64_t bytes_send;
    
    path = append_malloc(c->complete_cloud_directory, f_name);
    if(path == NULL)
    {
        WriteLog(c->log, 1, LOG_WARNING, "Could not allocate space for path");
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        return;
    }

    fd = open(path, O_RDONLY);

    WriteLog(c->log, 1, LOG_NOTICE, "Complete files path: [%s]", path);

    if(fd == -1)
    {
        free(path);
        SendBytes(c, NULL, 0, FILE_DOES_NOT_EXIST);
        WriteLog(c->log, 1, LOG_FAIL, "File [%s] does not exist - exiting...", path);
        return;
    }

    if(SendBytes(c, NULL, 0, PULL_FILE) == 0)
    {
        free(path);
        WriteLog(c->log, 1, LOG_WARNING, "Could not send bytes to client");
        close(fd);
        return;
    }

    bytes_send = SendFile_t(c, fd);
    if(bytes_send == 0) WriteLog(c->log, 1, LOG_FAIL, "Could not receive file because of socket error - [%lu] bytes were send", bytes_send);
    else WriteLog(c->log, 1, LOG_SUCCESS, "File was send successfully - [%lu] bytes were send", bytes_send);

    free(path);
    close(fd);
}
void List_File(Client *c)
{
    if(c == NULL) return;
    if(!c->Active) return;

    WriteLog(c->log, 1, LOG_NOTICE, "Client wants to list files");

    char *dir = (char *) malloc((NAME_MAX + 1) * sizeof(char));
    if(dir == NULL)
    {
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        WriteLog(c->log, 1, LOG_WARNING, "Could not allocate space for directory");
        return; 
    }

    char *dir_list = Get_Directory_List(c->complete_cloud_directory);
    if(dir_list == NULL)
    {
        free(dir);
        SendBytes(c, NULL, 0, ERROR_TOKEN);
        WriteLog(c->log, 1, LOG_FAIL, "Could not get directory list");
        return;
    }

    if(SendBytes(c, (uint8_t *) dir_list, strlen(dir_list), LIST_FILE) == 0)
    {
        free(dir);
        free(dir_list);
        WriteLog(c->log, 1, LOG_WARNING, "Could not send bytes to client");
        return;
    }

    WriteLog(c->log, 1, LOG_SUCCESS, "Directory list was send to client");

    free(dir);
    free(dir_list);

}

/* Deletes a file */
void Delete_File(Client *c, char *f_name)
{
    if(c == NULL || f_name == NULL) return;
    if(!c->Active) return;

    char *path;

    WriteLog(c->log, 1, LOG_NOTICE, "Client wants to delete file: [%s]", f_name);

    path = append_malloc(c->complete_cloud_directory, f_name);

    if(remove(path) == -1)
    {
        SendBytes(c, NULL, 0, FILE_DOES_NOT_EXIST);
        WriteLog(c->log, 1, LOG_FAIL, "Could not delete file: [%s] -> [%s]", path, strerror(errno));
        free(path);
        return;
    }

    SendBytes(c, NULL, 0, DELETE_FILE);
    WriteLog(c->log, 1, LOG_SUCCESS, "File: [%s] was deleted", f_name);

    free(path);
}
void Cat_File(Client *c, char *f_name)
{
    if(c == NULL || f_name == NULL) return;
    if(!c->Active) return;

    WriteLog(c->log, 1, LOG_NOTICE, "Client wants to cat file: [%s]", f_name);

    char *path;
    int fd;
    uint64_t bytes_send;

    path = append_malloc(c->complete_cloud_directory, f_name);

    fd = open(path, O_RDONLY);
    if(fd == -1)
    {
        free(path);
        SendBytes(c, NULL, 0, FILE_DOES_NOT_EXIST);
        WriteLog(c->log, 1, LOG_FAIL, "File [%s] does not exist", f_name);
        return;
    }

    if(SendBytes(c, NULL, 0, CAT_FILE) == 0)
    {
        free(path);
        WriteLog(c->log, 1, LOG_WARNING, "Could not send bytes to client");
        close(fd);
        return;
    }

    bytes_send = SendFile_t(c, fd);
    if(bytes_send == 0) WriteLog(c->log, 1, LOG_FAIL, "Could not send bytes because of socket error - [%lu] bytes were send", bytes_send);
    else WriteLog(c->log, 1, LOG_SUCCESS, "Cat of file [%s] was send - bytes send: [%lu]", f_name, bytes_send);

    free(path);
    close(fd);
}