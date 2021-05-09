#ifndef INTERFACE_H
#define INTERFACE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <linux/limits.h>
#include <libgen.h>
#include <sys/sendfile.h>
#include <fcntl.h>

typedef struct
{
    /* Threads */
    pthread_t front_end_thread;
    pthread_t back_end_thread;

    /* Thread synchro */
    int front_end_finished;
    int back_end_finished;

    /* Front end output */
    char *error;
    char *output;

    /* Front end input */
    int max_args;
    int max_arg_length;
    char **args;
    int number_of_arguments;

} Interface;

/* Interface */
Interface *Create_Interface();
void Delete_Interface(Interface *i);

int Set_Input(Interface *i, char *input);
void Output_Interface(Interface *i, char *s);

void Error_Interface(Interface *i, char *s);

#endif