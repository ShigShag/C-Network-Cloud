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
                sprintf(temp, "name: %s\tsize: %ld\n", dir->d_name, st.st_size);
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
int File_Exists(char *path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}