/* This file includes declarations of functions responsible for client side connection */

#ifndef CLIENT_CONNECTION
#define CLIENT_CONNECTION

#include "../common/includes.h"

/* receive server's default banner upon establishing connection */
void
recv_banner(SOCKET server_fd);                         

/* send client's username to the server */
void
send_username(SOCKET server_fd, char *username);        

/* send client's messages to the server */
void
send_msg(SOCKET server_fd);                             

/* receive messages coming from server */
void
recv_msg(SOCKET server_fd);                             

/* thread wrapper function for read_msg */
DWORD
WINAPI read_msg_thread(LPVOID lpParam);                 

#endif  /* CLIENT_CONNECTION */
