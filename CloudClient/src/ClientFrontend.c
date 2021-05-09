#include "../inc/ClientFrontend.h"
#include "../inc/ClientBackend.h"

void *Main_Routine_Front_End(void *arg)
{
    if(arg == NULL) return 0;

    Interface_arg *a = (Interface_arg *) arg;

    Client *c = a->client;
    Interface *i = a->interface;

    char input[1024];

    i->front_end_finished = 0;

    while(c->Active)
    {
        if(i->back_end_finished)
        {
            //system("clear");
            if(i->error != NULL)
            {
                /* Error Notice */
                if(i->error[strlen(i->error) - 1] == '\n'){
                    printf(RED "[-] %s" NRM, i->error);
                }
                else{
                    printf(RED "[-] %s\n" NRM, i->error);
                }
                Error_Interface(i, NULL);
            }

            if(i->output != NULL)
            {
                printf("%s\n", i->output);
                Output_Interface(i, NULL);
            }

            printf(">>>");
            fgets(input, 1024, stdin);
            input[strlen(input) - 1] = '\0';

            if(Set_Input(i, input) == 0)
            {
                Error_Interface(i, "Input failed exiting...\n");
                c->Active = 0;
            }

            i->back_end_finished  = 0;
            i->front_end_finished = 1;
            continue;
        }
        usleep(1000 * 100);
    }
    return 0;
}
char *Get_File_Path_Via_Dialog()
{
    char input[PATH_MAX + 1];

    printf("Enter file path:\n>>>");
    fgets(input, PATH_MAX + 1, stdin);
    input[strlen(input) - 1] = '\0';

    return realpath(input, NULL);
}