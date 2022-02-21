#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define TRUE		1
#define MSG_SIZE	256

void communicate(SOCKET);

int main(int argc, char *argv[]) {
	WSADATA	wsa;
	SOCKET socket_fd, client_fd;
	struct sockaddr_in server, client;
	int sockaddr_in_size;

	puts("[*] Initialising Winsock...\n");
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "[!!] Failed to initialze Winsock. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Winsock Initialisation complete.\n");

	if ( ( socket_fd = socket(AF_INET, SOCK_STREAM, 0) ) == INVALID_SOCKET) {
		fprintf(stderr, "[!!] Failed to create socket. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Socket Created.\n");

	server.sin_addr.s_addr = inet_addr("0.0.0.0");
	server.sin_family = AF_INET;
	server.sin_port = htons(7777);

	if ( bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Failed to bind address. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[*] Binded Address.\n");
	
	listen(socket_fd, 3);
	puts("[*] Listening on port 7777.\n");

	puts("[*] Accepting Connections.\n");
	
	sockaddr_in_size = sizeof(struct sockaddr_in);
	client_fd = accept(socket_fd, (struct sockaddr *)&client, &sockaddr_in_size);
	printf("[*] Accepted connection from: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	communicate(client_fd);

	getchar();

	closesocket(socket_fd);
	closesocket(client_fd);

	WSACleanup();

	return 0;

}

void communicate(SOCKET client_fd) {
	int recv_size = 0;
	char client_reply[MSG_SIZE];

	const char *banner = "Hello, There\nEnter message\nYou can exit by typing \":quit\"\n";
	send(client_fd, banner, strlen(banner), 0);

	while (TRUE) {
		recv_size = recv(client_fd, client_reply, 2000, 0);
		client_reply[recv_size] = '\0';

		puts("[DEBUG] Received Message.");
		printf("%s\n", client_reply);

		if (!strcmp(client_reply, "QUIT"))
			return;
	}
	
}