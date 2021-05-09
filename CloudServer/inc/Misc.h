#ifndef MISC_H
#define MISC_H

void Print_Help(char *argv);

char *Get_Directory_List(char *directory_path);

unsigned long Write_File(char *path, unsigned char *buffer, unsigned long size);

char* append_malloc(char* destination, char* source);
int append_realloc(char **dest, char *source);

int File_Exists(char *path);

#endif