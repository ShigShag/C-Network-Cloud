#ifndef MISC_H
#define MISC_H

void Print_Help(char *argv);

char *Get_Directory_List(char *directory_path);

unsigned long Write_File(char *path, unsigned char *buffer, unsigned long size);

char* append_malloc(char* destination, char* source);
int append_realloc(char **dest, char *source);

int check_value_buffer(unsigned char *buf, unsigned char value,unsigned int count);
char *convert_unsigend_char_to_hex(unsigned char *buf, unsigned int count);
int File_Exists(char *path);

#endif