/* This file includes definitions of macros and 
 * functions responsible for client side connection
 */


#include "interface.h"


/* Interface to operate on ncurses windows */
int     /* Return value: 1 - Success, 0 - Failure */
dumpchat_interface(int flag, char *buffer) {
    /* flag to determine action
     * buffer to store message in input/output operation
     */

    static WINDOW *dump_win;            /* to print network packet information */
    static WINDOW *input_win;           /* to get user input */
    static WINDOW *output_win;          /* to print server and user messages */          
    static WINDOW *banner_win;          /* to print application banner */

    if (flag == INTERFACE_INITIALIZE) {
        initscr();
        cbreak();
        
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        dump_win = newwin(max_y - 6, max_x / 2 - 8, 2, 8);
        scrollok(dump_win, TRUE);

        output_win = newwin(max_y / 2, max_x / 2 - 12, max_y - 40, max_x / 2 + 8);
        scrollok(output_win, TRUE);
        leaveok(output_win, TRUE);
        
        input_win = newwin(3, max_x / 2 - 12, max_y + max_y / 2 - 38, max_x / 2 + 8);

        banner_win = newwin(max_y / 3 - 4, max_x / 2 - 12, 2, max_x / 2 + 8);
        
        init_windows(dump_win, output_win, input_win, banner_win);
    } else if (flag == INTERFACE_INPUT && buffer != NULL) {
        input(input_win, buffer);
    } else if (flag == INTERFACE_OUTPUT && buffer != NULL) {
        output(output_win, buffer);
    } else if (flag == INTERFACE_CLEANUP) {
        endwin();
    } else {
        fprintf(stderr, "Error in operation. Please verify parameters passed.\n");
        return 0;
    }
    return 1;
}

/* Initialize windows and their default configuration */
void
init_windows(WINDOW *dump_win, WINDOW *output_win, WINDOW *input_win, WINDOW *banner_win) {
    /* dump_win, output_win, input_win, banner_win : handle to different windows on application interface
     * refer to interface() for further description
     */

    /* if command propmt supports color, enable colored output */
    if ( has_colors() ) {
        start_color();

        init_pair(DUMP_COLOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(OUTPUT_COLOR, COLOR_GREEN, COLOR_BLACK);
        init_pair(INPUT_COLOR, COLOR_RED, COLOR_BLACK);
        init_pair(BANNER_COLOR, COLOR_CYAN, COLOR_BLACK);

        wattron(dump_win, COLOR_PAIR(DUMP_COLOR));
        wattron(output_win, COLOR_PAIR(OUTPUT_COLOR));
        wattron(input_win, COLOR_PAIR(INPUT_COLOR));
        wattron(banner_win, COLOR_PAIR(BANNER_COLOR));
    }

    /* Initial dump_win window */
    wprintw(dump_win, "\n Dump here:\n");
    box(dump_win, 0, 0);
    wrefresh(dump_win);

    /* Initial output_win window */
    wprintw(output_win, "\n Output here:\n");
    box(output_win, 0, 0);
    wrefresh(output_win);

    /* Initial input_win window */
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "[>>>>] ");
    wrefresh(input_win);


    char *banner =
"\t\t      dMMMMb  dMP dMP dMMMMMMMMb dMMMMb  .aMMMb  dMP dMP .aMMMb dMMMMMMP\n\
\t\t    dMP VMP dMP dMP dMP dMP dMP dMP.dMP dMP VMP dMP dMP dMP dMP   dMP    \n\
\t\t   dMP dMP dMP dMP dMP dMP dMP dMMMMP  dMP     dMMMMMP dMMMMMP   dMP     \n\
\t\t  dMP.aMP dMP.aMP dMP dMP dMP dMP     dMP.aMP dMP dMP dMP dMP   dMP      \n\
\t\tdMMMMP   VMMMP  dMP dMP dMP dMP      VMMMP  dMP dMP dMP dMP   dMP "; 
    
    char *desc[] = {"\t\t[*] This application acts as a simple TCP-based chat client\n",
                    "\t\t[*] It also prints tcp packet information of incoming messages\n",
                    "\t\t[*] Developed by Aishwarya Raj.\n"};


    /* Banner window */
    wprintw(banner_win, "\n\n%s", banner);
    wprintw(banner_win, "\n\n\n\n%s%s%s", desc[0], desc[1], desc[2]);
    box(banner_win, 0, 0);
    wrefresh(banner_win);

}

/* Get user input from user-interface*/
void
input(WINDOW *input_win, char *str) {
    /* input_win: handle to input window
     * str: character array to store user input
     */
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "[>>>>] ");
    wrefresh(input_win);
    wgetstr(input_win, str);
    wclear(input_win);
}

/* Give output onto user-interface */
void
output(WINDOW *output_win, char *str) {
    /* output_win: handle to output window
     * str: character array to print on output window
     */

    wprintw(output_win, " %s\n", str);
    box(output_win, 0, 0);
    wrefresh(output_win);
}
