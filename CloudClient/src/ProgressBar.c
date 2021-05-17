#include "../inc/ProgressBar.h"
#include "../inc/ClientFrontend.h"
#include "malloc.h"
#include "string.h"

ProgressBar *Create_Progress_Bar(int id, char *name, char pad_symbol)
{
    ProgressBar *p = malloc(sizeof(ProgressBar));
    if(p == NULL) return NULL;

    p->id = id;
    p->name = NULL;
    p->progress = 0;
    p->pad_fill = "....................................................................................................";

    if(name != NULL)
    {
        p->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
        if(p->name == NULL)
        {
            free(p);
            return NULL;
        }

        strncpy(p->name, name, strlen(name) + 1);
    }

    p->pad = (char *) malloc(100 * sizeof(char));
    if(p->pad == NULL)
    {
        if(p->name != NULL) free(p->name);
        free(p);
        return NULL;
    } 
    
    memset(p->pad, pad_symbol, 100 * sizeof(char));

    return p;
}
void Delete_Progress_Bar(ProgressBar *b)
{
    if(b == NULL) return;

    if(b->name != NULL) free(b->name);
    if(b->pad != NULL) free(b->pad);
    free(b);
}
void Print_Progress_Bar(ProgressBar *b)
{
    if(b == NULL) return;

    if(b->progress < 0 || b->progress > 100) return;
    b->progress *= 100;
    printf("Progress %d%% " RED "[%.*s%.*s]" NRM "\r", (int) b->progress, (b->progress < 100) ? (int) b->progress : 0 ,b->pad, 100 - (int) b->progress, b->pad_fill);
    fflush(stdout);
}