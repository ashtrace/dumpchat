/* This file includes declarations of macros and functions
 * responsible for curses based user-interface
 */

#ifndef DUMPCHAT_INTERFACE_H
#define DUMPCHAT_INTERFACE_H

#include <curses.h>        /* for command-line interface */
#include <string.h>        /* for strlen */

/* color-pair identifier definition */
#define DUMP_COLOR      1   /* color-pair identifier of text in packet dump */
#define OUTPUT_COLOR    2   /* color-pair identifier of output text */
#define INPUT_COLOR     3   /* color-pair identifier of input text */
#define BANNER_COLOR    4   /* color-pair identifier of banner text */

#define INTERFACE_INITIALIZE    0     /* flag to perform initialization through interface function */
#define INTERFACE_INPUT         1     /* flag to perform input through interface function */
#define INTERFACE_OUTPUT        2     /* flag to perform output through interface function */
#define INTERFACE_STR_DUMP      3     /* flag to add packet data to respective curses window */
#define INTERFACE_RAW_DUMP      4     /* flag to print raw bytes to respective curses window */
#define INTERFACE_CLEANUP       5     /* flag to perform cleanup throught interface function */

typedef struct {		/* pass data information to raw_dump */
    unsigned int size;		/* size of raw data buffer */
    char *buffer;		/* pointer data buffer */
} raw_data_struct;

/* Interface to operate on ncurses windows */
int
dumpchat_interface(int flag, char *buffer);                        

/* Initialize windows and their default configuration */
void
init_windows(WINDOW *dump_win, WINDOW *output_win, WINDOW *input_win, WINDOW *banner_win);   

/* Get user input from user-interface*/
void                                
input(WINDOW *input_win, char *buffer);                                 

/* Give output onto user-interface */
void
output(WINDOW *output_win, char *buffer);                               

/* Give tcp packet information onto user-interface */
void
dump(WINDOW *dump_win, char *buffer);

/* Dump raw bytes */
void
raw_dump(WINDOW *dump_win, raw_data_struct *raw_data);

#endif /* DUMPCHAT_INTERFACE_H */
