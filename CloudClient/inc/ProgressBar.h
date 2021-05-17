#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

typedef struct
{
    int id;
    char *name;

    char *pad;
    char *pad_fill;

    double progress;
} ProgressBar;

ProgressBar *Create_Progress_Bar(int id, char *name, char pad_symbol);
void Delete_Progress_Bar(ProgressBar *b);

void Print_Progress_Bar(ProgressBar *b);

#endif 