/* This file forms the client side of dumpchat */

#include "connection.h"
#include "interface.h"
#include "../common/definitions.h"

int main(int argc, char *argv[]) {
	WSADATA             wsa;                    /* structure for initialising winsock */
	SOCKET              server_fd;              /* socket of server to connect to */
	struct sockaddr_in  server;                 /* structure to store server information for connection */
	char                *address;               /* IPv4 address to connect to */
	char                *port;                  /* port of service to connect to */
	char                *username;              /* username of client */
	DWORD               thread_id;              /* thread id of read_msg_thread */
	HANDLE              thread_handle;          /* handle to thread of read_msg_thread */
	HANDLE              process_heap_handle;    /* handle to process heap memory */

	puts("[DEBUG] Initialising interface...\n");
	dumpchat_interface(INTERFACE_INITIALIZE, NULL);

	puts("[DEBUG] Getting process Heap.\n");
	if ( (process_heap_handle = GetProcessHeap()) == NULL ) {
		fprintf(stderr, "[!!] Failed to get heap handler. Error Code: %d\n", GetLastError());
		return -1;
	}

	address 	= HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, ADDR_SIZE);
	port		= HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, 5);			        /* Max port: 65535 is 5 digits */
	username	= HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, USERNAME_SIZE);
    
    puts("[DEBUG] Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "[ERROR] Failed to initialze Winsock. Error code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Winsock Initialisation complete.\n");

	if ( ( server_fd = socket(AF_INET, SOCK_STREAM, 0) ) == INVALID_SOCKET ) {
		fprintf(stderr, "[ERROR] Failed to create socket. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Socket Created.\n");

	dumpchat_interface(INTERFACE_OUTPUT, "[INPUT] Enter server address:");
	dumpchat_interface(INTERFACE_INPUT, address);
	dumpchat_interface(INTERFACE_OUTPUT, "[INPUT] Enter port:");
	dumpchat_interface(INTERFACE_INPUT, port);
	dumpchat_interface(INTERFACE_OUTPUT, "[INPUT] Enter username:");
	dumpchat_interface(INTERFACE_INPUT, username);

	server.sin_addr.s_addr = inet_addr(address);
	server.sin_port = htons(atoi(port));
	server.sin_family = AF_INET;

	if ( connect(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		fprintf(stderr, "[ERROR] Failed to connect to %s:%s. Error Code: %d\n", address, port, WSAGetLastError());
		return -1;
	}

	printf("[DEBUG] Connected to %s:%s\n", address, port);

	recv_banner(server_fd);
	send_username(server_fd, username);

	thread_handle = CreateThread(NULL,
				     0,
				     read_msg_thread,
				     &server_fd,
				     0,
				     &thread_id);


	CloseHandle(thread_handle);
	send_msg(server_fd);

	HeapFree(process_heap_handle, 0, address);
	HeapFree(process_heap_handle, 0, username);
	HeapFree(process_heap_handle, 0, port);
	
	dumpchat_interface(INTERFACE_CLEANUP, NULL);

	return 0;
}
