#include "../inc/Client.h"
#include "../inc/Misc.h"

#define F_NULL_CHECK(val) if (val == NULL) return 0;

void Print_Help(char *argv)
{
    if(argv == NULL)
    {
        printf("[-] Could not print help -> argv was NULL\n");
        return;
    }

    printf("Usage: %s [CONFIG FILE PATH]\n", argv);
}
char *Get_Directory_List(char *directory_path)
{
    DIR *d;
    struct dirent *dir;
    struct stat st;
    char *list = NULL;
    char *path;
    char *temp = (char *) malloc(PATH_MAX * sizeof(char));
    if(temp == NULL) return NULL;

    d = opendir(directory_path);
    
    if(d)
    {
        sprintf(temp, "Data in directory:\n");
        append_realloc(&list, temp);

        while ((dir = readdir(d)) != NULL)
        {
            if(!strcmp(dir->d_name, ".")) continue;
            if(!strcmp(dir->d_name, "..")) continue;

            path = append_malloc(directory_path, dir->d_name);
            if(path == NULL)
            {
                closedir(d);
                return list;
            }

            if(stat(path, &st) == 0)
            {
                snprintf(temp, PATH_MAX * sizeof(char), "name: %s\tsize: %ld\n", dir->d_name, st.st_size);
                append_realloc(&list, temp);
            }
            free(path);
        }
        closedir(d);
        if(list == NULL)
        {
            append_realloc(&list, "Directory is empty\n");
        }
    }
    free(temp);
    return list;
}
unsigned long Write_File(char *path, unsigned char *buffer, unsigned long size)
{
    F_NULL_CHECK(buffer)

    FILE *fp;
    unsigned long BytesWritten;
    
    fp = fopen(path, "wb");
    F_NULL_CHECK(fp)

    BytesWritten = fwrite(buffer, sizeof(unsigned char), size, fp);

    fclose(fp);
    return BytesWritten;
}
int str_len(char* string)
{
    int length = 0;
    if(string != NULL)
    {
        while (*string != '\0')
        {
            length++;
            string++;
        }
    }
    return length;
}
char* append_malloc(char* destination, char* source)
{
    if(destination != NULL && source != NULL)
    {
        int i;
        int dest_size = str_len(destination) + 1;
        int source_size = str_len(source) + 1;
        char *temp_ptr = (char *) malloc((dest_size + source_size - 1) *sizeof(char));
        if(!temp_ptr)
        {
            return NULL;
        }
        for(i = 0;i < dest_size - 1;i++)
        {
            temp_ptr[i] = destination[i];
        }
        for(int n = 0;n < source_size;i++, n++)
        {
            temp_ptr[i] = source[n];
        }
        return temp_ptr;
    }
    return NULL;
}
int append_realloc(char **dest, char *source)
{
    char *temp;

    if(*dest != NULL && source != NULL)
    {
        temp = realloc(*dest, strlen(*dest) + strlen(source) + 1);

        if(temp == NULL)
        {
            return 0;
        }
        *dest = temp;
        strcpy(*dest + strlen(*dest), source);
    }
    else if(*dest == NULL && source != NULL)
    {
        temp = realloc(*dest, strlen(source) + 1);

        if(temp == NULL)
        {
            return 0;
        }
        *dest = temp;
        strcpy(*dest, source);
    }
    return 1;
}
int check_value_buffer(unsigned char *buf, unsigned char value, unsigned int count)
{
    if(buf == NULL) return 0;

    int passed = 1;
    for(int i = 0;i < count;i++){
        if(buf[i] != value){
            passed = 0;
            break;
        }
    }
    return passed;
}
char *convert_unsigend_char_to_hex(unsigned char *buf, unsigned int count)
{
    if(buf == NULL) return NULL;

    unsigned int size = (count * 2 + 1) * sizeof(char);
    char *h = (char *) malloc(size);
    if(h == NULL) return NULL;

    for(unsigned int i = 0;i < count;i++){
        snprintf(h + 2 * i, size, "%.2x", buf[i]);
    }
    return h;
}
int File_Exists(char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}
uint8_t *Uint32ToUint8(uint32_t value)
{
    uint8_t *Array = (uint8_t *) calloc(sizeof (uint32_t), sizeof (uint8_t));
    if(Array)
    {
        for(uint32_t i = 0;i < sizeof (uint32_t);i++)
        {
            Array[i] = (value >> ((sizeof (uint32_t) * 8) - ((i + 1) * 8))) & 0xFF;
        }
    }
    return Array;
}
uint32_t Uint8ToUint32(const uint8_t *ByteArray)
{
    uint32_t value = 0;
    uint32_t i;

    if(ByteArray != NULL)
    {
        for (i = 0; i < sizeof (uint32_t) - 1; i++)
        {
            value = (value | ByteArray[i]) << 8;
        }
        value = (value | ByteArray[i]);
    }
    return value;
}
uint8_t *Uint64ToUint8(uint64_t value)
{
    uint8_t *Array = (uint8_t *) calloc(sizeof (uint64_t), sizeof (uint8_t));
    if(Array)
    {
        for(uint64_t i = 0;i < sizeof (uint64_t);i++)
        {
            Array[i] = (value >> ((sizeof (uint64_t) * 8) - ((i + 1) * 8))) & 0xFF;
        }
    }
    return Array;
}
uint64_t Uint8ToUint64(const uint8_t *ByteArray)
{
    uint64_t value = 0;
    uint32_t i;

    if(ByteArray != NULL)
    {
        for (i = 0; i < sizeof (uint64_t) - 1; i++)
        {
            value = (value | ByteArray[i]) << 8;
        }
        value = (value | ByteArray[i]);
    }
    return value;
}
void free_memset(void *buf, unsigned long count)
{
    if(buf == NULL) return;

    memset(buf, 0, count);
    free(buf);
}
char ascii_to_hex(int num)
{
        if(num < 58 && num > 47)
        {
                return num - 48; 
        }
        if(num < 103 && num > 96)
        {
                return num - 87;
        }
        return (char) num;
}