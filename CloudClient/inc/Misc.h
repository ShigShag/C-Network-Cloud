#ifndef MISC_H
#define MISC_H

void PrintHelp(char *p_name);

/* File read/write */
int Read_File(char *path, unsigned char **buffer, unsigned long *bytes_read);
unsigned long Write_File(char *path, unsigned char *buffer, unsigned long size);

#endif
