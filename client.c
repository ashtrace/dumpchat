#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define TRUE			1
#define ADDR_SIZE		16
#define MSG_SIZE		256
#define USR_NAME_SIZE	32

void recv_banner(SOCKET server_fd);
void send_username(SOCKET server_fd, char *username);
void communicate(SOCKET server_fd);

int main(int argc, char *argv[]) {
	WSADATA wsa;
	SOCKET socket_fd;
	struct sockaddr_in server;
	char address[ADDR_SIZE];
	char username[USR_NAME_SIZE];
	unsigned int port = 0;

	puts("[DEBUG] Initialising Winsock...\n");

	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "[ERROR] Failed to initialze Winsock. Error code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Winsock Initialisation complete.\n");

	if ( ( socket_fd = socket(AF_INET, SOCK_STREAM, 0) ) == INVALID_SOCKET ) {
		fprintf(stderr, "[ERROR] Failed to create socket. Error Code: %d\n", WSAGetLastError());
		return -1;
	}

	puts("[DEBUG] Socket Created.\n");

	printf("[+] Enter server address: ");
	scanf("%s", address);
	printf("[+] Enter port: ");
	scanf("%d", &port);
	printf("[+] Enter username: ");
	scanf("%s", username);

	server.sin_addr.s_addr = inet_addr(address);
	server.sin_port = htons(port);
	server.sin_family = AF_INET;

	if ( connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		fprintf(stderr, "[ERROR] Failed to connect.\n to %s:%d. Error Code: %d\n", address, port, WSAGetLastError());
		return -1;
	}

	printf("[DEBUG] Connected to %s:%d\n", address, port);

	recv_banner(socket_fd);
	send_username(socket_fd, username);
	communicate(socket_fd);

	return 0;
}

void recv_banner(SOCKET server_fd) {
	char server_banner[MSG_SIZE] = {0};
	int recv_size = 0;

	if ((recv_size = recv(server_fd, server_banner, 2000, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "[ERROR] Failed to receive data.\n Error Code: %d\nPlease Retry.\n", WSAGetLastError());
		send(server_fd, "QUIT", strlen("QUIT"), 0);
		exit(-1);
	} else if (recv_size == 0) {
		puts("[DEBUG] Connection closed.");
	}
	server_banner[recv_size] = '\0';
	printf("[DEBUG] Received Data.\n%s\n", server_banner);
}

void send_username(SOCKET server_fd, char *username) {
	int recv_size = 0;
	char conf_msg [4] = {0};

	if (send(server_fd, username, strlen(username), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[ERROR] Could not send username to the server. Error code: %d\n", WSAGetLastError());
		exit(-1);
	}

	recv_size = recv(server_fd, conf_msg, sizeof(conf_msg), 0);
	if (recv_size == 0 || !strcmp(conf_msg, "CONF")) {
		fprintf(stderr, "[ERROR] Server did not receive username. Aborting.\n");
		exit(-1);
	}
}

void communicate(SOCKET server_fd) {
	char server_reply[MSG_SIZE], message[MSG_SIZE] = {0};

	int recv_size = 0;

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