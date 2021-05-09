#include "../inc/Server.h"
#include "../inc/Misc.h"
#include "../inc/Config.h"
int main(int argc ,char *argv[])
{
    if(argc != 2) 
    {
        Print_Help(argv[0]);
        return 1;
    }

    Server *s = Create_Server(argv[1]);
    if(s) StartServer(s);

    
    return 0;
}