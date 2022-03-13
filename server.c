#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define TRUE			1
#define MSG_SIZE		256
#define USERNAME_SIZE	32
#define ADDR_SIZE		16

DWORD WINAPI communication_thread(LPVOID lpParam);

void send_banner(SOCKET client_socket, char *addr);
void recv_username(SOCKET client_socket, char *username);
void communicate(SOCKET client_socket, char *addr, char *username);

typedef struct {
	SOCKET client_socket;
	char username[USERNAME_SIZE];
	char client_addr[ADDR_SIZE];
} client_param_struct;

int main(int argc, char *argv[]) {
	WSADATA	wsa;
	SOCKET server_socket;
	struct sockaddr_in server, client;
	const unsigned int sockaddr_in_size = sizeof(struct sockaddr_in);
	const unsigned int client_param_struct_size = sizeof(client_param_struct);
	client_param_struct *client_param;
	DWORD thread_id;
	HANDLE thread_handle;
	HANDLE process_heap_handle;

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

	server.sin_addr.s_addr = inet_addr("0.0.0.0");
	server.sin_family = AF_INET;
	server.sin_port = htons(7777);

	if ( bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Failed to bind address. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Binded Address.\n");
	
	listen(server_socket, 3);
	puts("[DEBUG] Listening on port 7777.\n");

	puts("[DEBUG] Accepting Connections.\n");
	
	while ( TRUE ) {

		client_param = (client_param_struct *) HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, client_param_struct_size);

		if ((client_param -> client_socket = accept(server_socket, (struct sockaddr *) &client, &sockaddr_in_size)) == INVALID_SOCKET) {
			fprintf(stderr, "[!!] Accept failed. Error Code: %d\n", WSAGetLastError());
			ExitProcess(-1);
		}

		printf("[DEBUG] Accepted connection from: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		/* Get IP address of the client connection */
		strcpy(client_param -> client_addr, inet_ntoa(client.sin_addr));

		thread_handle = CreateThread(NULL,						/* default security attributes */
									0,							/* default stack size */
									communication_thread,		/* thread function */
									client_param,				/* arguement to thread function */
									0,							/* default creation flags */
									&thread_id);
		
		CloseHandle(thread_handle);}

	getchar();

	closesocket(server_socket);

	WSACleanup();

	return 0;

}

DWORD WINAPI communication_thread(LPVOID lpParam) {
	client_param_struct *client_param = (client_param_struct *) lpParam;

	/* Send banner to client */
	send_banner(client_param -> client_socket, client_param -> client_addr);

	/* Receive user name from client */
	recv_username(client_param -> client_socket, client_param -> username);

	/* Communicate with the client */
	communicate(client_param -> client_socket, client_param -> client_addr, client_param -> username);

	ExitThread(0);
}

void send_banner(SOCKET client_socket, char *addr) {
	const char *banner = "Hello, There\nEnter message\nYou can exit by typing \":quit\"\n";
	if (send(client_socket, banner, strlen(banner), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Could not send banner to %s. Error code: %d\n", addr, WSAGetLastError());
		ExitThread(-1);
	}
}

void recv_username(SOCKET client_socket, char *username) {
	int recv_size = 0;
	if ( (recv_size = recv(client_socket, username, USERNAME_SIZE, 0) ) == SOCKET_ERROR) {
	fprintf(stderr, "[DEBUG] Error in receiving username. Error Code: %d\n", WSAGetLastError());
	ExitThread(-1);
	}
	printf("[DEBUG] Received username: %s.\n", username);
}

void communicate(SOCKET client_socket, char *addr, char *username) {
	int recv_size = 0;
	char client_reply[MSG_SIZE];
	
	while (TRUE) {		
		if ( (recv_size = recv(client_socket, client_reply, MSG_SIZE, 0) ) == SOCKET_ERROR) {
			fprintf(stderr, "[DEBUG] Error in receiving message. Error Code: %d\n", WSAGetLastError());
			ExitThread(-1);
		}
		client_reply[recv_size] = '\0';

		puts("[DEBUG] Received Message.");
		printf("[%s@%s] %s\n", username, addr, client_reply);

		if (!strcmp(client_reply, "QUIT")) {
			closesocket(client_socket);
			return;
		}
	}
	
}