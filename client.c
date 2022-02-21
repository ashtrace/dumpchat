#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define TRUE		1
#define ADDR_SIZE	16
#define MSG_SIZE	256

void communicate(SOCKET);

int main(int argc, char *argv[]) {
	WSADATA wsa;
	SOCKET socket_fd;
	struct sockaddr_in server;
	char address[ADDR_SIZE];
	unsigned int port = 0;

	puts("[*] Initialising Winsock...\n");

	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "[!!] Failed to initialze Winsock. Error code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Winsock Initialisation complete.\n");

	if ( ( socket_fd = socket(AF_INET, SOCK_STREAM, 0) ) == INVALID_SOCKET ) {
		fprintf(stderr, "[!!] Failed to create socket. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Socket Created.\n");

	printf("[+] Enter server address: ");
	scanf("%s", address);
	printf("[+] Enter port: ");
	scanf("%d", &port);

	server.sin_addr.s_addr = inet_addr(address);
	server.sin_port = htons(port);
	server.sin_family = AF_INET;

	if ( connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		fprintf(stderr, "[!!] Failed to connect.\n to %s:%d. Error Code: %d\n", address, port, WSAGetLastError());
		return -1;
	}

	printf("[*] Connected to %s:%d\n", address, port);

	communicate(socket_fd);

	return 0;
}

void communicate(SOCKET server_fd) {
	char server_reply[MSG_SIZE], message[MSG_SIZE] = {0};

	int recv_size = 0;

	if ((recv_size = recv(server_fd, server_reply, 2000, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Failed to receive data.\n Error Code: %d\n", WSAGetLastError());
	} else if (recv_size == 0) {
		puts("[*] Connection closed.");
	}
	server_reply[recv_size] = '\0';
	printf("[*] Received Data.\n%s\n", server_reply);

	setbuf(stdin, NULL);
	while (TRUE) {
		printf("[>>>>] ");
		fgets(message, MSG_SIZE, stdin);

		if (!strcmp(message, ":quit\n")) {
			send(server_fd, "QUIT", strlen("QUIT"), 0);
			return;
		} else {
			send(server_fd, message, strlen(message), 0);
		}
	}
}

	