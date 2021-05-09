#include "../inc/ClientBackend.h"
#include "../inc/ClientFrontend.h"
#include "../inc/Misc.h"

int main (int argc, char *argv[])
{
    if(argc != 2)
    {
        PrintHelp(argv[0]);
        return 1;
    }

    Client *c = Create_Client(argv[1]);
    if(c == NULL)
    {
        perror("Could not create Client\n");
        return 1;
    }

    Interface *i = Create_Interface();

    if(i == NULL)
    {
        free(c->config);
        free(c);
        perror("Could not create interface\n");
        return 1;
    }

    int err;
    Interface_arg *arg = malloc(sizeof(Interface_arg));
    if(arg == NULL)
    {
        perror("Could not create Interface arg\n");
        Delete_Client(c);
        Delete_Interface(i);
        return 1;
    }

    arg->client = c;
    arg->interface = i;
    err = pthread_create(&i->back_end_thread, NULL, &Main_Routine_Back_End, arg);
    if(err != 0)
    {
        c->Active = 0;
        printf("[-] Could not start client backend thread: %s\n", strerror(err));
    }
    err = pthread_create(&i->front_end_thread, NULL, &Main_Routine_Front_End, arg);
    if(err != 0)
    {
        c->Active = 0;
        printf("[-] Could not start client front thread: %s\n", strerror(err));
    }

    pthread_join(i->front_end_thread, NULL);
    printf("Front end joined\n");
    pthread_join(i->back_end_thread, NULL);
    printf("Back end joined\n");

    Delete_Interface(i);

    free(arg);
    return 0;
}
