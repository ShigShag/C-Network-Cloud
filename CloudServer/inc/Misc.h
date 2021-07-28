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


uint8_t *Uint32ToUint8(uint32_t value);
uint32_t Uint8ToUint32(const uint8_t *ByteArray);
uint8_t *Uint64ToUint8(uint64_t value);
uint64_t Uint8ToUint64(const uint8_t *ByteArray);

void free_memset(void *buf, unsigned long count);
char ascii_to_hex(int num);
#endif