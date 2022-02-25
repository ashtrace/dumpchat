#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define TRUE			1
#define MSG_SIZE		256
#define USER_NAME_SIZE	32

void send_banner(SOCKET client_socket, char *addr);
void recv_username(SOCKET client_socket, char *username);
void communicate(SOCKET client_socket, char *addr, char *username);

int main(int argc, char *argv[]) {
	WSADATA	wsa;
	SOCKET server_socket, client_socket;
	struct sockaddr_in server, client;
	const int sockaddr_in_size = sizeof(struct sockaddr_in);
	char username[USER_NAME_SIZE];

	puts("[*] Initialising Winsock...\n");
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "[!!] Failed to initialze Winsock. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Winsock Initialisation complete.\n");

	if ( ( server_socket = socket(AF_INET, SOCK_STREAM, 0) ) == INVALID_SOCKET) {
		fprintf(stderr, "[!!] Failed to create socket. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Socket Created.\n");

	server.sin_addr.s_addr = inet_addr("0.0.0.0");
	server.sin_family = AF_INET;
	server.sin_port = htons(7777);

	if ( bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Failed to bind address. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Binded Address.\n");
	
	listen(server_socket, 3);
	puts("[*] Listening on port 7777.\n");

	puts("[*] Accepting Connections.\n");
	
	while ( (client_socket = accept(server_socket, (struct sockaddr *)&client, &sockaddr_in_size)) != INVALID_SOCKET) {
		printf("[*] Accepted connection from: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		/* Send banner to client */
		send_banner(client_socket, inet_ntoa(client.sin_addr));

		/* Receive user name from client */
		recv_username(client_socket, username);

		/* Start communication with client */
		communicate(client_socket, inet_ntoa(client.sin_addr), username);
	}

	if (client_socket == INVALID_SOCKET) {
		fprintf(stderr, "[!!] Accept failed. Error Code: %d\n", WSAGetLastError());
	}

	getchar();

	closesocket(server_socket);

	WSACleanup();

	return 0;

}

void send_banner(SOCKET client_socket, char *addr) {
	const char *banner = "Hello, There\nEnter message\nYou can exit by typing \":quit\"\n";
	if (send(client_socket, banner, strlen(banner), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Could not send banner to %s.Error code: %d\n", addr, WSAGetLastError());
		exit(-1);
	}
}

void recv_username(SOCKET client_socket, char *username) {
	int recv_size = 0;
	recv_size = recv(client_socket, username, USER_NAME_SIZE, 0);
	username[recv_size] = '\0';
	if (recv_size) {
		send(client_socket, "CONF", strlen("CONF"), 0);
		printf("[DEBUG] Received username: %s.\n", username);
	}
	else {
		send(client_socket, "QUIT", strlen("QUIT"), 0);
		fprintf(stderr, "[!!] Could not receive username %s.Error code: %d\n", WSAGetLastError());
		exit(-1);
	}
}

void communicate(SOCKET client_socket, char *addr, char *username) {
	int recv_size = 0;
	char client_reply[MSG_SIZE];
	
	while (TRUE) {
		recv_size = recv(client_socket, client_reply, 2000, 0);
		client_reply[recv_size] = '\0';

		puts("[DEBUG] Received Message.");
		printf("[%s@%s] %s\n", username, addr, client_reply);

		if (!strcmp(client_reply, "QUIT")) {
			closesocket(client_socket);
			return;
		}
	}
	
}