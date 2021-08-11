#include "../inc/Logging.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* Logging priorities ascii*/
#define LOG_NOTICE_ASCII "[*]"
#define LOG_WARNING_ASCII "[!]"
#define LOG_SUCCESS_ASCII "[+]"
#define LOG_FAIL_ASCII "[-]"

/* Colors */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

/* Create the logger struct */
Log *CreateLogger(char *log_file_path)
{
    Log *l = (Log *) malloc(sizeof(Log));
    if(l == NULL) return NULL;

    // Create file if not exist
    FILE *fp = fopen(log_file_path, "a+");
    if(fp == NULL){
        free(l);
        return NULL;
    }
    fclose(fp);

    strncpy(l->log_path, log_file_path, sizeof(l->log_path));
    return l;
}

/* Write log */
int WriteLog(Log *l, int ConsoleOutput, int prioritie, const char *format, ...)
{
    if(l == NULL && ConsoleOutput == 0) return 0;

    FILE *fp = NULL;

    if(l){
        fp = fopen(l->log_path, "a");
        if(fp == NULL && ConsoleOutput == 0) return 0;
    }

    va_list arg;
    va_list arg_cp;

    int bytes_written = 0;

    char time_info[100];
    time_t t;
    struct tm *ts;

    t = time(NULL);
    ts = localtime(&t);

    // Format time string
    strftime(time_info, sizeof(time_info), "%x - %I:%M:%S %p", ts);

    va_start(arg, format);

    // Output priority
    switch (prioritie)
    {
    case LOG_NOTICE:
        if(fp != NULL) fprintf(fp, "%s ", LOG_NOTICE_ASCII);
        if(ConsoleOutput == 1) fprintf(stdout, "%s ", LOG_NOTICE_ASCII);
        break;

    case LOG_WARNING:
        if(fp != NULL) fprintf(fp, "%s ", LOG_WARNING_ASCII);
        if(ConsoleOutput == 1) fprintf(stdout, KRED "%s " KNRM, LOG_WARNING_ASCII);
        break;

    case LOG_SUCCESS:
        if(fp != NULL) fprintf(fp, "%s ", LOG_SUCCESS_ASCII);
        if(ConsoleOutput == 1) fprintf(stdout, "%s ",  LOG_SUCCESS_ASCII);
        break;

    case LOG_FAIL:
        if(fp != NULL) fprintf(fp, "%s ", LOG_FAIL_ASCII);
        if(ConsoleOutput == 1) fprintf(stdout, KRED "%s " KNRM, LOG_FAIL_ASCII);
        break;
    
    default:
        if(fp != NULL) fprintf(fp, "%s ", LOG_NOTICE_ASCII);
        if(ConsoleOutput == 1) fprintf(stdout, "%s ", LOG_NOTICE_ASCII);
        break;
    }

    // Write the time
    if(fp != NULL) fprintf(fp, "%s\t", time_info);
    if(ConsoleOutput == 1) fprintf(stdout, "%s\t", time_info);

    // Write to console 
    if(ConsoleOutput == 1){
        va_copy(arg_cp, arg);
        vfprintf(stdout, format, arg_cp);

        // Write new line character
        fprintf(stdout, "\n");
        va_end(arg_cp);
    }

    // Write actual message and write new line character
    if(fp != NULL) {
        bytes_written = vfprintf(fp, format, arg);
        fprintf(fp, "\n");
    }

    va_end(arg);

    if(fp != NULL) fclose(fp);
    return bytes_written;
}