/* This file includes definitions of components responsible for client side connection */

#include "connection.h"

/* wrapper thread function for client communication functions */
DWORD       /* standard format requires DWORD return type, although we terminate by closing the thread through ExitThread */
WINAPI communication_thread(LPVOID lpParam) {
    /* lpParam: pointer to thread's parameter */

	client_struct **ptr_to_client_list_head = ( (thread_param_struct *) lpParam ) -> ptr_to_client_list_head;       /* pointer to pointer to head (front) of linked list of clients */
	client_struct **ptr_to_client_list_tail = ( (thread_param_struct *) lpParam ) -> ptr_to_client_list_tail;       /* pointer to pointer to tail (rear) linked list of clients */
	client_struct *current_client = ( (thread_param_struct *) lpParam ) -> current_client;                          /* pointer to current client */

	/* Send banner to client */
	if ( send_banner(current_client -> client_socket, current_client -> client_addr) ) {
		remove_client(current_client, ptr_to_client_list_head, ptr_to_client_list_tail);
		ExitThread(1);
	}

	/* Receive user name from client */
	if ( recv_username(current_client -> client_socket, current_client -> username) ) {
		remove_client(current_client, ptr_to_client_list_head, ptr_to_client_list_tail);
		ExitThread(1);
	}

	/* Communicate with the client */
	communicate(current_client, *ptr_to_client_list_head);

	/* Delete the client from the client list */
	remove_client(current_client, ptr_to_client_list_head, ptr_to_client_list_tail);

	/* Terminate this communication thread */
	ExitThread(0);
}

/* send server's banner to client */
int     /* Return value: 0 - Succes, 1 - Failure */
send_banner(SOCKET client_socket, char *addr) {
	const char *banner = "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=\n Hello, There\n Enter message\n You can exit by typing \":quit\"\n =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=\n";

	if (send(client_socket, banner, strlen(banner), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Could not send banner to %s. Error code: %d\n", addr, WSAGetLastError());
		return 1;
	}
	return 0;
}

/* receive username from client */
int     /* Return value: 0 - Succes, 1 - Failure */
recv_username(SOCKET client_socket, char *username) {
	int recv_size = 0;

	if ( (recv_size = recv(client_socket, username, USERNAME_SIZE, 0) ) == SOCKET_ERROR) {
		fprintf(stderr, "[DEBUG] Error in receiving username. Error Code: %d\n", WSAGetLastError());
		return 1;
	}
	printf("[DEBUG] Received username: %s.\n", username);
	return 0;
}

/* assist communication between all connected clients */
void
communicate(client_struct * client, client_struct *client_list_head) {
	int             recv_size;                  /* size of received message */
	char            client_reply[MSG_SIZE];     /* message from client */
	char            message[2 * MSG_SIZE];      /* formatted message to broadcast to all connected clients */
	client_struct   *client_node;               /* client from which message is received */

    recv_size = 0;

	while (TRUE) {		
		if ( (recv_size = recv(client -> client_socket, client_reply, MSG_SIZE, 0) ) == SOCKET_ERROR) {
			fprintf(stderr, "[DEBUG] Error in receiving message. Error Code: %d\n", WSAGetLastError());
			return ;
		}
		client_reply[recv_size] = '\0';

		puts("[DEBUG] Received Message.");
		printf("[%s@%s] %s\n", client -> username, client -> client_addr, client_reply);
		sprintf(message , "[%s@%s] %s", client -> username, client -> client_addr, client_reply);

		if (!strcmp(client_reply, "QUIT")) {
			closesocket(client -> client_socket);
			return ;
		}

		/* broadcast message to all clients present in the client list */
		client_node = client_list_head;
		while (client_node) {
			send(client_node -> client_socket, message, strlen(message), 0);
			printf("[DEBUG] Sent message to %d: %s.\n", client_node -> client_socket, message);
			client_node = client_node -> next;
		}
	}
	
}

/* add new clients to linked list */
client_struct *
append_client(client_struct **head, client_struct **tail, HANDLE process_heap_handle) {
	const unsigned int client_struct_size = sizeof(client_struct);
	client_struct *new_client = HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, client_struct_size);

	if ( new_client == NULL ) {
		fprintf(stderr, "[DEBUG] Could not allocate memory for client. Error code: %d.\n", GetLastError());
		ExitProcess(-1);
	}

	new_client -> next = NULL;

	if ( *tail ) {
		( *tail ) -> next = new_client;
		new_client -> prev = *tail;
		*tail = new_client;
	} else {
		new_client -> prev = NULL;
		*head = new_client;
		*tail = new_client;
	}

	return new_client;
}

/* remove disconnected clients from linked list */
int     /* Return value: 0 - Succes, 1 - Failure */
remove_client(client_struct *client, client_struct **head, client_struct **tail) {
	HANDLE process_heap_handle = GetProcessHeap();
	
	if ( process_heap_handle == NULL ) {
		fprintf(stderr, "[!!] Failed to get heap handler while removing client. Unable to remove client. Error Code: %d\n", GetLastError());
		return 1;
	}
	
	if ( client ) {
		printf("[DEBUG] Removing client %d.\n", client -> client_socket);

		if ( client -> prev ) {
			client -> prev -> next = client -> next;
		} else {
			*head = client -> next;
		}

		if ( client -> next ) {
			client -> next -> prev = client -> prev;
		} else {
			*tail = client -> prev;
		}

		HeapFree(process_heap_handle, 0, client);

		return 0;
	}
}
