#include "../inc/Interface.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Window */
#define MAIN_MENU 1
#define CLIENT_LIST 2

/* Menu Points */
#define LIST 0
#define EXIT 1

Interface *CreateInterface()
{
    //if(s == NULL) return NULL;

    Interface *i = (Interface *) malloc(sizeof(Interface));
    if(i == NULL) return NULL;

    i->active = 1;

    return i;
}
void DeleteInterface(Interface *i)
{
    if(i == NULL) return;

    i->active = 0;

    pthread_join(i->MainThread, NULL);

    free(i);
}
void *InterfaceMainRoutine(void *i_)
{
    if(i_ == NULL) return 0;

    Interface *i = (Interface *) i_;
    initscr();
    noecho();
    cbreak();

    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);

    int current_window = MAIN_MENU;

    /* Main menu */
    char *choices[] = {"List", "Exit"};
    int choice_size = sizeof(choices) / sizeof(choices[0]);
    int choice_max = choice_size - 1;
    int menu_highlighted = 0;
    int choice;
    WINDOW *menu = newwin(choice_size + 2, xmax - 12, ymax - 8, 5);
    box(menu, 0, 0);
    wrefresh(menu);

    /* Client list */
    WINDOW *client_list = newwin(ymax - 10, xmax - 12, 0 + (ymax * 0.05), 5);
    int client_highlighted = 0;
    box(client_list, 0, 0);

    while(i->active)
    {
        wrefresh(client_list);
        wrefresh(menu);
        
        /* print menu contents*/
        for(int n = 0;n < choice_size;n++){
            if(n == menu_highlighted && current_window == MAIN_MENU) wattron(menu, A_REVERSE);
            mvwprintw(menu, n + 1, 1, "%s", choices[n]);
            wattroff(menu, A_REVERSE);
        }

        /* print clients */
        /* ACHTUNG: POTENTIELLER SEGMENTATION FAULT, WENN DIE CLIENT LIST NICHT GELOCKT WIRD */
        for(int n = 0;n < *i->clients_connected;n++){
            if(n == client_highlighted && current_window == CLIENT_LIST) wattron(client_list, A_REVERSE);
            mvwprintw(client_list, n + 1, 1, "[%s]", i->client_list[n]->ip);
            wattroff(client_list, A_REVERSE); 
        }

        if(current_window == MAIN_MENU)
        {
            choice = wgetch(menu);

            switch(choice)
            {
            case 'q':
                if(menu_highlighted > 0) menu_highlighted--;
                break;

            // Temporary
            case ' ':
                i->active = 0;
                    break;

            case 'e':
                if(menu_highlighted < choice_max) menu_highlighted++;
                break;

            case 'f':
                switch(menu_highlighted)
                {
                    case(EXIT):
                        i->active = 0;
                        break;

                    case(LIST):
                        current_window = CLIENT_LIST;
                        break;

                    default:
                        break;
                }
            break;

            default:
                break;
            }
        }else if(current_window == CLIENT_LIST)
        {
            choice = wgetch(client_list);

            switch(choice){
                // Go back to menu
                case 'c':
                    current_window = MAIN_MENU;
                    break;

                case 'q':
                    if(client_highlighted > 0) client_highlighted--;
                    break;

                case 'e':
                    if(client_highlighted < *i->clients_connected) client_highlighted++;
                    break;
                
            }
        }
    }
    

    endwin();

    return 0;
}