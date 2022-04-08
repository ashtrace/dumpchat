#include "tcpsniffer.h"

DWORD
WINAPI init_sniffer_thread(LPVOID lpParam) {
    if ( init_sniffer((char *)lpParam) ) {
        fprintf(stderr, "[!!] Failed to initialize sniffer.\n");
        ExitThread(1);
    }
    ExitThread(0);
}

int init_sniffer(char *address) {
    SOCKET              sniffer;
    struct sockaddr_in  dest;
    WSADATA             wsa;
    int                 bytes_returned;

    /* Initialize winsock */
    if ( WSAStartup(MAKEWORD(2, 2), &wsa) != 0 ) {
        fprintf(stderr, "WSAStartup() failed.\n");
        return 1;
    }

    /* Create a rwa socket */
    if ( ( sniffer = socket(AF_INET, SOCK_RAW, IPPROTO_IP) ) == INVALID_SOCKET ) {
        fprintf(stderr, "Failed to create raw socket.\n");
        return 1;
    }

    ZeroMemory(&dest, sizeof(dest));
    dest.sin_addr.s_addr = inet_addr(address);
    dest.sin_family = AF_INET;
    dest.sin_port = 0;          /* any random port */

    /* Binding socket to local system */
    if ( bind(sniffer, (struct sockaddr *)&dest, sizeof(dest)) == SOCKET_ERROR ) {
        fprintf(stderr, "bind(%s) failed\n", address);
        return 1;
    }

    /* Enable socket to receive all */
    int j = 1;

    if ( WSAIoctl(sniffer, SIO_RCVALL, &j, sizeof(j), 0, 0, (LPDWORD) &bytes_returned, 0, 0) == SOCKET_ERROR ) {
        fprintf(stderr, "WSAIoctl failed\n");
        return 1;
    }

    /* Start sniffing */
    start_sniffing(sniffer, address);

    /* End */
    closesocket(sniffer);
    WSACleanup();

    return 0;
}

void
start_sniffing(SOCKET sniffer, char *address) {
    char    *buffer;
    int     size;

    if ( (buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SNIFFED_BUFFER_SIZE)) == NULL) {
        fprintf(stderr, "HeapAalloc() failed.\n");
        return;
    }

    while ( TRUE ) {
        if ( ( size = recvfrom(sniffer, buffer, SNIFFED_BUFFER_SIZE, 0, 0, 0) ) > 0 ) {
            process_packet(address, buffer, size);
        } else {
            fprintf(stderr, "recvfrom() failed.Error code: %d\n", WSAGetLastError());
            break;
        }
    }

    HeapFree(GetProcessHeap(), 0, buffer);
}

void
process_packet(char *address, char *buffer, unsigned int size) {
    if ( ((ip_hdr *)buffer ) -> ip_protocol == TCP_PROTOCOL && ((ip_hdr *)buffer ) -> ip_srcaddr == inet_addr(address)) {
            print_tcp_packet(buffer, size);
    }
}

void
print_ip_header(char *buffer) {
    ip_hdr                  *iphdr;
    unsigned short          iphdrlen;
    struct in_addr          src_addr, dest_addr;
    char                    *dump_msg;

    dump_msg = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DUMP_BUFFER_SIZE);
    
    iphdr              = (ip_hdr *)buffer;
    iphdrlen           = iphdr -> ip_header_len * 4;
    src_addr.s_addr    = iphdr -> ip_srcaddr;
    dest_addr.s_addr   = iphdr -> ip_destaddr;

    dumpchat_interface(INTERFACE_STR_DUMP, "\n IP Header:\n");

    sprintf(dump_msg, "\t|-IP Version: %17u\n", iphdr -> ip_version);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-IP Header Length: %11u bytes\n", iphdr -> ip_header_len);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Type Of Service: %12u\n", iphdr -> ip_tos);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-IP Packet Length: %11u bytes\n", ntohs(iphdr -> ip_total_length));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Identification: %13u\n", ntohs(iphdr -> ip_id));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Reserved Zero Field: %8u\n", iphdr -> ip_reserved_zero);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Don't Fragment Field: %7u\n", iphdr -> ip_dont_fragment);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-More Fragment Field: %8u\n", iphdr -> ip_more_fragment);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Time To Live: %15u\n", iphdr -> ip_ttl);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Protocol: %19u\n", iphdr -> ip_protocol);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Checksum: %19u\n", ntohs(iphdr -> ip_checksum));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Source IP Address: %s\n", inet_ntoa(src_addr));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Destination IP Address: %s\n", inet_ntoa(dest_addr));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);

    HeapFree(GetProcessHeap(), 0, dump_msg);
}

void
print_tcp_packet(char *buffer, unsigned int size) {
    unsigned short      iphdrlen;
    tcp_hdr             *tcphdr;
    char                *dump_msg;

    dump_msg    = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DUMP_BUFFER_SIZE);
    iphdrlen    = ((ip_hdr *)buffer ) -> ip_header_len * 4;
    tcphdr      = (tcp_hdr *)(buffer + iphdrlen);

    dumpchat_interface(INTERFACE_STR_DUMP, "\n******************************************************************************************\n");

    print_ip_header(buffer);

    dumpchat_interface(INTERFACE_STR_DUMP, "\n TCP Header:\n");
    sprintf(dump_msg, "\t|-Source Port: %16u\n", ntohs(tcphdr -> source_port));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Destination Port: %11u\n", ntohs(tcphdr -> dest_port));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Sequence Number: %12u\n", ntohl(tcphdr -> sequence));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Acknowledge Number: %9u\n", ntohl(tcphdr -> acknowledge));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Header Length: %14u bytes\n", tcphdr -> data_offset * 4);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-CWR Flag: %19u\n", tcphdr -> cwr);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-ECN Flag: %19u\n", tcphdr -> ecn);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Urgent Flag: %16u\n", tcphdr -> urg);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Acknowledgement Flag: %7u\n", tcphdr -> ack);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Push Flag: %18u\n", tcphdr -> psh);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Reset Flag: %17u\n", tcphdr -> rst);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Synchronise Flag: %11u\n", tcphdr -> syn);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Finish Flag: %16u\n", tcphdr -> fin);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Window: %21u\n", ntohs(tcphdr -> window));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Checksum: %19u\n", ntohs(tcphdr -> checksum));
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);
    sprintf(dump_msg, "\t|-Urgent Pointer: %13u\n", tcphdr -> urgent_pointer);
    dumpchat_interface(INTERFACE_STR_DUMP, dump_msg);

    dumpchat_interface(INTERFACE_STR_DUMP, "\n RAW DATA:\n");

    dumpchat_interface(INTERFACE_STR_DUMP, " IP Header:\n");
    print_data(buffer, iphdrlen);
    
    dumpchat_interface(INTERFACE_STR_DUMP, " TCP Header:\n");
    print_data(buffer + iphdrlen, tcphdr -> data_offset * 4);

    dumpchat_interface(INTERFACE_STR_DUMP, " Data Payload:\n");
    print_data(buffer + iphdrlen + tcphdr -> data_offset * 4, ( size - iphdrlen - tcphdr -> data_offset * 4));

    dumpchat_interface(INTERFACE_STR_DUMP, "\n******************************************************************************************\n");

    HeapFree(GetProcessHeap(), 0, dump_msg);
}


void print_data(char *data, unsigned int size) {
    char *buffer;
    raw_data_struct *raw_data;

    buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    CopyMemory(buffer, data, size);

    raw_data -> size = size;
    raw_data -> buffer = buffer;

    dumpchat_interface(INTERFACE_RAW_DUMP, (char *) raw_data);

    HeapFree(GetProcessHeap(), 0, buffer);
}
