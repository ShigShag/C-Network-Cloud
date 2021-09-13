void Receive_File_Fast(Client *c, char *f_name)
{
    if(c == NULL) return;
    if(!c->Active) return;

    char *path;
    FILE *fp;

    path = append_malloc(c->complete_cloud_directory, f_name);

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


/* Transmission Client Thread */
void *Receive_Packet_t(void *arg);

/* Resets transmission client array */
void Reset_Client_Transmission_Array(Client *c);
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