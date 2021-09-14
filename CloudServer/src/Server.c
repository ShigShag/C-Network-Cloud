#include "../inc/Server.h"
#include "../inc/Communication.h"
#include "../inc/Misc.h"
#include "../inc/ClientManagment.h"
#include "../inc/Logging.h"

Server *Create_Server(char *config_file_path)
{
    Server *s = malloc(sizeof(Server));
    if(s == NULL) 
    {
        printf("Failed to Allocate Sapce for Server: %s", strerror(errno));
        return NULL;
    }

    s->config = Get_Config(config_file_path);
    if(s->config == NULL)
    {
        free(s);
        return NULL;    
    }

    // Create Server environment 
    CreateServerEnvionment(s);

    s->log = CreateLogger(s->config->server_log_path);
    if(s->log == NULL) 
    {
        WriteLog(s->log, 1, LOG_FAIL, "Could not open Log file");
    }
    else
    {   
        WriteLog(s->log, 1, LOG_SUCCESS, "Writing log file at: [%s]", s->log->log_path);
    }

    s->CLIENT = (Client **) malloc(s->config->max_clients * sizeof(Client));
    if(s->CLIENT == NULL)
    {
        WriteLog(s->log, 1, LOG_FAIL, "Failed to Allocate Sapce for Client: %s", strerror(errno));
        free(s);
        return NULL;
    }

    memset(&s->addr, 0, sizeof(s->addr));
    s->addr.sin_family = AF_INET;
    s->addr.sin_port = htons(s->config->port);
    s->addr.sin_addr.s_addr = INADDR_ANY;

    s->max_clients = s->config->max_clients;
    s->clients_connected = 0;
    s->port = s->config->port;
    s->Client_Lock = 0;

    // Create Cloud Folder if not exists
    mkdir(s->config->cloud_directory, 0744);

    WriteLog(s->log, 1, LOG_SUCCESS, "Address set");

    return s;
}
int Create_Socket(Server *s, int receive_timeout_sec)
{
    int option = 1;
    int err;
    struct timeval tv;

    if((s->Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        WriteLog(s->log, 1, LOG_FAIL, "Could not create Socket: %s", strerror(errno));
        return 0;
    }
    setsockopt(s->Socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    
    if(receive_timeout_sec > 0)
    {
        tv.tv_sec = receive_timeout_sec;
        tv.tv_usec = 0;
        err = setsockopt(s->Socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
        if(err != 0)
        {
            WriteLog(s->log, 1, LOG_FAIL, "Could not set receive timeout: %s", strerror(errno));
        }else
        {
            WriteLog(s->log, 1, LOG_FAIL, "Timeout set to %d seconds", receive_timeout_sec);
        }
    }

    WriteLog(s->log, 1, LOG_SUCCESS, "Created socket");


    if(bind(s->Socket, (struct sockaddr*) &s->addr, sizeof(s->addr)) == -1)
    {
        WriteLog(s->log, 1, LOG_FAIL, "[-] Could not bind Socket: %s", strerror(errno));
        return 0;
    }

    WriteLog(s->log, 1, LOG_SUCCESS, "Binded socket");
    return 1;
}
int StartServer(Server *s)
{
    if(s == NULL){
        printf("[-] Server was NULL\n");
        return 0;
    }
    
    if(!Create_Socket(s, s->config->receive_timeout)) return 0;

    int err;
    s->Activated = 1;

    err = pthread_create(&s->ListeningThread, NULL, &ClientListeningThread, s);
    if(err != 0){
        s->Activated = 0;
        WriteLog(s->log, 1, LOG_FAIL, "Could not start listening thread: %s", strerror(err));
    }

    err = pthread_create(&s->ClientMonitorThread, NULL, &ClientListMonitor, s);
    if(err != 0){
        s->Activated = 0;
        WriteLog(s->log, 1, LOG_FAIL, "Could not start client monitor thread: %s", strerror(err));
    }

    WriteLog(s->log, 1, LOG_NOTICE, "MAX_CLIENTS: %d", s->config->max_clients);

    if(!s->Activated)
    {
        DeleteServer(s);
        return 0;
    }
    
    getchar();

    DeleteServer(s);
    return 1;
}
void DeleteServer(Server *s)
{
    if(s == NULL) return;
    s->Activated = 0;

    RemoveAllClients(s);
    // TODO Terminate Threads

    /*DWORD w = WaitForSingleObject(s->h_ListeningThread, 1000);
    if(w == WAIT_TIMEOUT) TerminateThread(s->h_ListeningThread, 0);

    w = WaitForSingleObject(s->h_ClientMonitor, 1000);
    if(w == WAIT_TIMEOUT) TerminateThread(s->h_ListeningThread, 0);*/
    close(s->Socket);
    free(s->CLIENT);
    WriteLog(s->log, 1, LOG_NOTICE, "Server deleted");
    free(s->log);
    Delete_Config(s->config);
    free(s);
}
void CreateServerEnvionment(Server *s)
{
    if(s == NULL) return;

    char *dir;
    int fd;

    /* directory for saving the files of the clients */
    mkdir_recursive(s->config->cloud_directory, 0700);
    WriteLog(s->log, 1, LOG_NOTICE, "Created directory: [%s]", s->config->cloud_directory);

    /* directory for saving clients logs */
    if(s->config->client_log_directory != NULL)
    {
        mkdir_recursive(s->config->client_log_directory, 0700);
        WriteLog(s->log, 1, LOG_NOTICE, "Created directory: [%s]", s->config->client_log_directory);
    }

    /* directory for server logs */
    if(s->config->server_log_path != NULL)
    {
        dir = get_directory_name(s->config->server_log_path);
        if(dir != NULL)
        {
            mkdir_recursive(dir, 0700);
            WriteLog(s->log, 1, LOG_NOTICE, "Created directory: [%s]", dir);
            free(dir);
        }
        fd = open(s->config->server_log_path, O_CREAT, S_IRUSR | S_IWUSR);
        WriteLog(s->log, 1, LOG_NOTICE, "Created file: [%s]", s->config->server_log_path);
        close(fd);
    }

    /* directory for saving client authentication */
    dir = get_directory_name(s->config->client_credentials_path);
    if(dir != NULL)
    {
        mkdir_recursive(dir, 0700);
        WriteLog(s->log, 1, LOG_NOTICE, "Created directory: [%s]", dir);
        free(dir);            
    }

    fd = open(s->config->client_credentials_path, O_CREAT, S_IRUSR | S_IWUSR);
    WriteLog(s->log, 1, LOG_NOTICE, "Created file: [%s]", s->config->client_credentials_path);
    close(fd);

    /* directory for saving clients id´s */
    dir = get_directory_name(s->config->client_database_path);
    if(dir != NULL)
    {
        mkdir_recursive(dir, 0700);
        WriteLog(s->log, 1, LOG_NOTICE, "Created directory: [%s]", dir);
        free(dir);
    }
    fd = open(s->config->client_database_path, O_CREAT, S_IRUSR | S_IWUSR);
    WriteLog(s->log, 1, LOG_NOTICE, "Created file: [%s]", s->config->client_database_path);
    close(fd);
}