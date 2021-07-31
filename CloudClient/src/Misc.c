#include "../inc/Misc.h"
#include "../inc/ClientBackend.h"
#include <termios.h>

void PrintHelp(char *p_name)
{
    if(p_name)
        printf("\nUsage: %s [IP] [PORT]\n", p_name);
}

/* File read/write */
int Read_File(char *path, unsigned char **buffer, unsigned long *bytes_read)
{
    if(path == NULL) return -1;

    FILE *fp;
    unsigned long size;

    fp = fopen(path, "rb");
    if(fp == NULL) return -1;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    *buffer = (unsigned char *) malloc(size * sizeof(unsigned char));

    *bytes_read = fread(*buffer, sizeof(unsigned char), size, fp);

    fclose(fp);

    return *bytes_read == size;
}
unsigned long Write_File(char *path, unsigned char *buffer, unsigned long size)
{
    if(buffer == NULL) return 0;

    FILE *fp;
    unsigned long bytes_written;
    
    fp = fopen(path, "wb");
    if(fp == NULL) return 0;

    bytes_written = fwrite(buffer, sizeof(unsigned char), size, fp);

    fclose(fp);
    return bytes_written;
}
char *RetrievePassword(unsigned int max_size, unsigned int *count){
   if(max_size == 0) return NULL;

   char *pw = malloc(max_size * sizeof(char));
   if(pw == NULL) return NULL;

   struct termios term;

   // Get configuration
   tcgetattr(fileno(stdin), &term);
   term.c_lflag &= ~ECHO;

   // Set new configuration
   tcsetattr(fileno(stdin), 0, &term);

   fgets(pw, max_size * sizeof(char), stdin);

   // Reset configuration
   term.c_lflag |= ECHO;
   tcsetattr(fileno(stdin), 0, &term);

   *count = strlen(pw);

   return pw;
}
void FreePassword(char *password, unsigned int *count)
{
    if(password == NULL || count == NULL) return;
    memset(password, 0, *count);
    free(password);
    *count = 0;
}
