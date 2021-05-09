#ifndef CLIENT_FRONTEND_H
#define CLIENT_FRONTEND_H

#define NRM  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\x1B[36m"
#define WHT  "\x1B[37m"

/* Front end routine */
void *Main_Routine_Front_End(void *arg);

char *Get_File_Path_Via_Dialog();

#endif