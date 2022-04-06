#ifndef DUMPCHAT_INTERFACE_H
#define DUMPCHAT_INTERFACE_H

#include <curses.h>


/* color-pair identifier definition */
#define DUMP_COLOR      1   /* color-pair identifier of text in packet dump */
#define OUTPUT_COLOR    2   /* color-pair identifier of output text */
#define INPUT_COLOR     3   /* color-pair identifier of input text */
#define BANNER_COLOR    4   /* color-pair identifier of banner text */

#define INTERFACE_INITIALIZE    0     /* flag to perform initialization through interface function */
#define INTERFACE_INPUT         1     /* flag to perform input through interface function */
#define INTERFACE_OUTPUT        2     /* flag to perform output through interface function */
#define INTERFACE_CLEANUP       3     /* flag to perform cleanup throught interface function */

int
dumpchat_interface(int, char *);                        /* Interface to operate on ncurses windows */

void
init_windows(WINDOW *, WINDOW *, WINDOW *, WINDOW *);   /* Initialize windows and their default configuration */

void                                
input(WINDOW *, char *);                                /* Get user input from user-interface*/ 

void
output(WINDOW *, char *);                               /* Give output onto user-interface */

#endif /* DUMPCHAT_INTERFACE_H */
