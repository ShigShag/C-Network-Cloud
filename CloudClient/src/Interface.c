#include  "../inc/Interface.h"

Interface *Create_Interface()
{
    Interface *i = (Interface *) malloc(sizeof(Interface));
    if(i == NULL)
    {
        printf("[-] Could not allocate memory for Interface: %s\n", strerror(errno));
        return NULL;
    }
    i->max_args = 10;
    i->max_arg_length = PATH_MAX + 1;
    i->args = (char **) malloc(i->max_args * sizeof(char *));

    for(int n = 0;n < i->max_args;n++)
    {
        i->args[n] = (char *) malloc((i->max_arg_length) * sizeof(char));
        if(i->args[n] == NULL)
        {
            printf("[-] Could not allocate memory for Interface: %s\n", strerror(errno));
            for(int x = 0;x < n;n++)
            {
                free(i->args[x]);
                free(i);
            }
            return NULL;            
        }
    }

    if(i->args == NULL)
    {
        printf("[-] Could not allocate memory for argument input: %s\n", strerror(errno));
        free(i);
        return NULL;
    }

    i->back_end_finished = 0;
    i->front_end_finished = 0;
    i->error = NULL;
    i->output = NULL;

    return i;
} 
void Delete_Interface(Interface *i)
{
    if(i == NULL) return;

    free(i->error);
    free(i->output);
    for(int n = 0;n < i->max_args;n++)
    {
        free(i->args[n]);
    }
    free(i->args);
    free(i);
}
int Set_Input(Interface *i, char *input)
{
    if(i == NULL || input == NULL) return 0;

    char *token;
    i->number_of_arguments = 0;

    token = strtok(input, " ");

    while(token != NULL && i->number_of_arguments < i->max_args)
    {
        strncpy(i->args[i->number_of_arguments], token, strlen(token) + 1);

        i->number_of_arguments ++;
        token = strtok(NULL, " ");
    }
    return 1;
}
void Output_Interface(Interface *i, char *s)
{
    if(i == NULL) return;

    if(i->output != NULL) free(i->output);

    if(s == NULL)
    {
        i->output = NULL;
        return;
    }else
    {
        i->output = (char *) malloc((strlen(s) + 1) * sizeof(char));
        if(i->output == NULL) return;
        strncpy(i->output, s, (strlen(s) + 1) * sizeof(char));
    }
}
void Error_Interface(Interface *i, char *s)
{
    if(i == NULL) return;

    if(i->error != NULL) free(i->error);

    if(s == NULL)
    {
        i->error = NULL;
        return;
    }else
    {
        i->error = (char *) malloc((strlen(s) + 1) * sizeof(char));
        if(i->error == NULL) return;
        strncpy(i->error, s, (strlen(s) + 1) * sizeof(char));
    }
}