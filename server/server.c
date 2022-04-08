/* This file forms the server side of dumpchat */

#include "connection.h"

#define BIND_ADDRESS    "0.0.0.0"
#define BIND_PORT       7777

int main(int argc, char *argv[]) {
	WSADATA                 wsa;                                                        /* structure for initialising winsock */
	SOCKET                  server_socket;                                              /* socket of binded server */
	client_struct           *current_client, *client_list_head, *client_list_tail;      /* pointer to current client, head and tail of linked list of clients */
	struct sockaddr_in      server, client;                                             /* information for binding and connection of server and client rrespectively*/
	unsigned int            sockaddr_in_size;                                           /* size of structure sockaddr_in */
	DWORD                   thread_id;                                                  /* id of thread of client communication */
	HANDLE                  thread_handle;                                              /* handle to thread of client communication */
	HANDLE                  process_heap_handle;                                        /* handle to process heap memory */
	thread_param_struct     *thread_param;                                              /* parameter to thread of client communication */

	sockaddr_in_size  = sizeof(struct sockaddr_in);
    
	/* Initialize client list */
	client_list_head = NULL;
	client_list_tail = NULL;

	puts("[DEBUG] Getting process Heap.\n");
	if ( (process_heap_handle = GetProcessHeap()) == NULL ) {
		fprintf(stderr, "[!!] Failed to get heap handler. Error Code: %d\n", GetLastError());
		return -1;
	}

	puts("[DEBUG] Initialising Winsock...\n");
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "[!!] Failed to initialze Winsock. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Winsock Initialisation complete.\n");

	if ( ( server_socket = socket(AF_INET, SOCK_STREAM, 0) ) == INVALID_SOCKET) {
		fprintf(stderr, "[!!] Failed to create socket. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Socket Created.\n");

	server.sin_addr.s_addr = inet_addr(BIND_ADDRESS);
	server.sin_family = AF_INET;
	server.sin_port = htons(BIND_PORT);

	if ( bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Failed to bind address. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Binded Address.\n");
	
	listen(server_socket, 3);
	puts("[DEBUG] Listening on port 7777.\n");

	puts("[DEBUG] Accepting Connections.\n");
	
	while ( TRUE ) {

		thread_param = HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, sizeof(thread_param_struct));

		current_client = append_client(&client_list_head, &client_list_tail, process_heap_handle);
		thread_param -> ptr_to_client_list_head = &client_list_head;
		thread_param -> ptr_to_client_list_tail = &client_list_tail;

		printf("[DEBUG] Got client_list_head after client_append() as: 0x%x.\n", client_list_head);
		
		if ((current_client -> client_socket = accept(server_socket, (struct sockaddr *) &client, &sockaddr_in_size)) == INVALID_SOCKET) {
			fprintf(stderr, "[!!] Accept failed. Error Code: %d\n", WSAGetLastError());
			ExitProcess(-1);
		}

		printf("[DEBUG] Accepted connection from: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		printf("[DEBUG] Current client: %d.\n", current_client -> client_socket);

		/* Get IP address of the client connection */
		strcpy(current_client -> client_addr, inet_ntoa(client.sin_addr));

		thread_param -> current_client = current_client;

		/* Create thread to handle the client */
		thread_handle = CreateThread(NULL, 0, communication_thread, thread_param, 0, &thread_id);
		
		CloseHandle(thread_handle);
	}

	getchar();

	closesocket(server_socket);

	WSACleanup();

	return 0;

}
