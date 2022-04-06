#include <stdio.h>
#include <winsock2.h>
#include "interface.h"

#pragma comment(lib, "ws2_32.lib")

#define TRUE			1
#define ADDR_SIZE		16
#define MSG_SIZE		256
#define USERNAME_SIZE		32

DWORD WINAPI read_msg_thread(LPVOID lpParam);

void recv_banner(SOCKET server_fd);
void send_username(SOCKET server_fd, char *username);
void send_msg(SOCKET server_fd);
void read_msg(SOCKET server_fd);

void show_output(char *message);

int main(int argc, char *argv[]) {
	WSADATA wsa;
	SOCKET server_fd;
	struct sockaddr_in server;
	char *address;
	char *port;
	char *username;
	DWORD thread_id;
	HANDLE thread_handle;
	HANDLE process_heap_handle;

	puts("[DEBUG] Initialising interface...\n");
	dumpchat_interface(INTERFACE_INITIALIZE, NULL);
	puts("[DEBUG] Initialized interface...\n");

	puts("[DEBUG] Initialising Winsock...\n");

	puts("[DEBUG] Getting process Heap.\n");
	if ( (process_heap_handle = GetProcessHeap()) == NULL ) {
		fprintf(stderr, "[!!] Failed to get heap handler. Error Code: %d\n", GetLastError());
		return -1;
	}

	address 	= HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, ADDR_SIZE);
	port		= HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, 5);			/* Max port: 65535 is 5 digits */
	username	= HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, USERNAME_SIZE);

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
		fprintf(stderr, "[ERROR] Failed to connect.\n to %s:%s. Error Code: %d\n", address, port, WSAGetLastError());
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

void recv_banner(SOCKET server_fd) {
	char *server_banner;
	int recv_size = 0;

	server_banner = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MSG_SIZE);

	if ((recv_size = recv(server_fd, server_banner, 2000, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "[ERROR] Failed to receive data.\n Error Code: %d. Please Retry.\n", WSAGetLastError());
		ExitProcess(1);
	} else if (recv_size == 0) {
		dumpchat_interface(INTERFACE_OUTPUT, "[DEBUG] Connection closed.");
		dumpchat_interface(INTERFACE_CLEANUP, NULL);
		HeapFree(GetProcessHeap(), 0, server_banner);
		ExitProcess(1);
	}

	server_banner[recv_size] = '\0';
	dumpchat_interface(INTERFACE_OUTPUT, server_banner);
	HeapFree(GetProcessHeap(), 0, server_banner);
}

void send_username(SOCKET server_fd, char *username) {
	if (send(server_fd, username, strlen(username), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[ERROR] Could not send username to the server. Error code: %d\n", WSAGetLastError());
		dumpchat_interface(INTERFACE_CLEANUP, NULL);
		ExitProcess(1);
	}
}

void send_msg(SOCKET server_fd) {
	char *message;

	message = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MSG_SIZE);

	int recv_size = 0;

	while (TRUE) {

		dumpchat_interface(INTERFACE_INPUT, message);

		if (!strcmp(message, ":quit")) {
			send(server_fd, "QUIT", strlen("QUIT"), 0);
			HeapFree(GetProcessHeap(), 0, message);
			dumpchat_interface(INTERFACE_CLEANUP, NULL);
			return;
		} else {
			send(server_fd, message, strlen(message), 0);
		}
	}
}

DWORD WINAPI read_msg_thread(LPVOID lpParam) {
	read_msg(*(SOCKET *)lpParam);
	dumpchat_interface(INTERFACE_CLEANUP, NULL);
	ExitThread(0);
}

void read_msg(SOCKET server_fd) {
	int recv_size;
	char *server_reply;

	server_reply = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MSG_SIZE);

	while ( ( recv_size = recv(server_fd, server_reply, MSG_SIZE, 0) ) != SOCKET_ERROR ) {
		server_reply[recv_size] = '\0';
		dumpchat_interface(INTERFACE_OUTPUT, server_reply);
	}
	if (recv_size == SOCKET_ERROR) {
		puts("[DEBUG] Could not receive message from server. Exiting process.");
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, server_reply);
		dumpchat_interface(INTERFACE_CLEANUP, NULL);
		ExitProcess(1);
	}
}
