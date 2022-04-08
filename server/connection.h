/* This file includes declarations of components responsible for server side connection */

#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H

#include "../common/includes.h"
#include "../common/definitions.h"

typedef struct client_structure {				        /* node in linked list of clients */
	SOCKET                      client_socket;			/* socket to client's connection */
	char	                    username[USERNAME_SIZE];		/* username sent by client */
	char	                    client_addr[ADDR_SIZE];		/* IPv4 address of client */
	struct client_structure     *prev, *next;		        /* pointers to previous and next client-nodes in linked list of clients */
} client_struct;

typedef struct {                                	/* store client parameters to communication_thread function */
	client_struct **ptr_to_client_list_head;	/* pointer to pointer to head (front) of linked list of clients */
	client_struct **ptr_to_client_list_tail;	/* pointer to pointer to tail (rear) linked list of clients */
	client_struct *current_client;			/* pointer to current client */
} thread_param_struct;

/* wrapper thread function for client communication functions */
DWORD
WINAPI communication_thread(LPVOID lpParam);            

/* send server's banner to client */
int
send_banner(SOCKET, char *);

/* receive username from client */
int
recv_username(SOCKET client_socket, char *username);

/* assist communication between all connected clients */
void
communicate(client_struct *client, client_struct *client_list_head);

/* add new clients to linked list */
client_struct *
append_client(client_struct **head, client_struct **tail, HANDLE process_heap_handle);

/* remove disconnected clients from linked list */
int 
remove_client(client_struct *client, client_struct **head, client_struct **tail);

#endif /* SERVER_CONNECTION_H */
