#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define TRUE				1
#define MSG_SIZE			256
#define USERNAME_SIZE			32
#define ADDR_SIZE			16

typedef struct client_structure {
	SOCKET client_socket;
	char username[USERNAME_SIZE];
	char client_addr[ADDR_SIZE];
	struct client_structure *prev, *next;
} client_struct;

typedef struct {
	client_struct **ptr_to_client_list_head;
	client_struct **ptr_to_client_list_rear;
	client_struct *current_client;
} thread_param_struct;

DWORD WINAPI communication_thread(LPVOID lpParam);

int send_banner(SOCKET client_socket, char *addr);
int recv_username(SOCKET client_socket, char *username);
void communicate(client_struct *client, client_struct *client_list_head);

client_struct * append_client(client_struct **head, client_struct **rear, HANDLE process_heap_handle);
int remove_client(client_struct *client, client_struct **head, client_struct **rear);

int main(int argc, char *argv[]) {
	WSADATA	wsa;
	SOCKET server_socket;
	client_struct *current_client, *client_list_head, *client_list_rear;
	struct sockaddr_in server, client;
	const unsigned int sockaddr_in_size = sizeof(struct sockaddr_in);
	const unsigned int client_struct_size = sizeof(client_struct);
	DWORD thread_id;
	HANDLE thread_handle;
	HANDLE process_heap_handle;
	thread_param_struct *thread_param;

	/* Initialize client list */
	client_list_head = NULL;
	client_list_rear = NULL;

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

		thread_param = HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, sizeof(thread_param_struct));

		current_client = append_client(&client_list_head, &client_list_rear, process_heap_handle);
		thread_param -> ptr_to_client_list_head = &client_list_head;
		thread_param -> ptr_to_client_list_rear = &client_list_rear;

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

		thread_handle = CreateThread(NULL,				/* default security attributes */
					     0,					/* default stack size */
					     communication_thread,		/* thread function */
					     thread_param,			/* arguement to thread function */
					     0,					/* default creation flags */
					     &thread_id);
		
		CloseHandle(thread_handle);
	}

	getchar();

	closesocket(server_socket);

	WSACleanup();

	return 0;

}

DWORD WINAPI communication_thread(LPVOID lpParam) {
	client_struct **ptr_to_client_list_head = ( (thread_param_struct *) lpParam ) -> ptr_to_client_list_head;
	client_struct **ptr_to_client_list_rear = ( (thread_param_struct *) lpParam ) -> ptr_to_client_list_rear;
	client_struct *current_client = ( (thread_param_struct *) lpParam ) -> current_client;

	/* Send banner to client */
	if ( send_banner(current_client -> client_socket, current_client -> client_addr) ) {
		remove_client(current_client, ptr_to_client_list_head, ptr_to_client_list_rear);
		ExitThread(-1);
	}

	/* Receive user name from client */
	if ( recv_username(current_client -> client_socket, current_client -> username) ) {
		remove_client(current_client, ptr_to_client_list_head, ptr_to_client_list_rear);
		ExitThread(-1);
	}

	/* Communicate with the client */
	communicate(current_client, *ptr_to_client_list_head);

	/* Delete the client from the client list */
	remove_client(current_client, ptr_to_client_list_head, ptr_to_client_list_rear);

	/* Terminate this communication thread */
	ExitThread(0);
}

int send_banner(SOCKET client_socket, char *addr) {
	const char *banner = "Hello, There\nEnter message\nYou can exit by typing \":quit\"\n";
	if (send(client_socket, banner, strlen(banner), 0) == SOCKET_ERROR) {
		fprintf(stderr, "[!!] Could not send banner to %s. Error code: %d\n", addr, WSAGetLastError());
		return -1;
	}
	return 0;
}

int recv_username(SOCKET client_socket, char *username) {
	int recv_size = 0;
	if ( (recv_size = recv(client_socket, username, USERNAME_SIZE, 0) ) == SOCKET_ERROR) {
		fprintf(stderr, "[DEBUG] Error in receiving username. Error Code: %d\n", WSAGetLastError());
		return -1;
	}
	printf("[DEBUG] Received username: %s.\n", username);
	return 0;
}

void communicate(client_struct * client, client_struct *client_list_head) {
	int recv_size = 0;
	char client_reply[MSG_SIZE];
	char message[2 * MSG_SIZE];
	client_struct *client_node;

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

client_struct * append_client(client_struct **head, client_struct **rear, HANDLE process_heap_handle) {
	const unsigned int client_struct_size = sizeof(client_struct);
	client_struct *new_client = HeapAlloc(process_heap_handle, HEAP_ZERO_MEMORY, client_struct_size);

	if ( new_client == NULL ) {
		fprintf(stderr, "[DEBUG] Could not allocate memory for client. Error code: %d.\n", GetLastError());
		ExitProcess(-1);
	}

	new_client -> next = NULL;

	if ( *rear ) {
		( *rear ) -> next = new_client;
		new_client -> prev = *rear;
		*rear = new_client;
	} else {
		new_client -> prev = NULL;
		*head = new_client;
		*rear = new_client;
	}

	return new_client;
}

int remove_client(client_struct *client, client_struct **head, client_struct **rear) {
	HANDLE process_heap_handle = GetProcessHeap();
	
	if ( process_heap_handle == NULL ) {
		fprintf(stderr, "[!!] Failed to get heap handler while removing client. Error Code: %d\n", GetLastError());
		return -1;
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
			*rear = client -> prev;
		}

		HeapFree(process_heap_handle, 0, client);

		return 0;
	}
}
