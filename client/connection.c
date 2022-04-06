/* This file includes definitions of functions responsible for client side connection */

#include "connection.h"
#include "interface.h"
#include "../common/definitions.h"

/* receive server's default banner upon establishing connection */
void
recv_banner(SOCKET server_fd) {
	/* server_fd: socket of server to receive banner from */
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

/* send client's username to the server */
void
send_username(SOCKET server_fd, char *username) {
	/* server_fd: socket of server to receive message from
	 * username: username input by client to be used in chat
	 */
	if (send(server_fd, username, strlen(username), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[ERROR] Could not send username to the server. Error code: %d\n", WSAGetLastError());
		dumpchat_interface(INTERFACE_CLEANUP, NULL);
		ExitProcess(1);
	}
}

/* send client's messages to the server */
void
send_msg(SOCKET server_fd) {
	/* server_fd: socket of server to send message from */
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

/* receive messages coming from server */
void
read_msg(SOCKET server_fd) {
	/* server_fd: socket of server to receive message from */
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

/* thread wrapper function for read_msg */
DWORD		/* standard format requires DWORD return type, although we terminate by closing the thread through ExitThread */
WINAPI read_msg_thread(LPVOID lpParam) {
	read_msg(*(SOCKET *)lpParam);
	dumpchat_interface(INTERFACE_CLEANUP, NULL);
	ExitThread(0);
}
