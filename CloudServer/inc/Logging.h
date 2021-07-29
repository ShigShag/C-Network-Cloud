#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <limits.h>

/* Logging priorities codes*/
#define LOG_NOTICE 0x1
#define LOG_WARNING 0x2
#define LOG_SUCCESS 0x3
#define LOG_FAIL 0x4

typedef struct
{
    /* File Handler */
    char log_path[PATH_MAX + 1];
} Log;

/* Create the logger struct */
Log *CreateLogger(char *log_file_path); 

/* Write log */
int WriteLog(Log *l, int ConsoleOutput, int prioritie, const char *format, ...);

#endif