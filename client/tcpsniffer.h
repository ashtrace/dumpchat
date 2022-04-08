#ifndef TCP_SNIFFER_H
#define TCP_SNIFFER_H

#include <winsock2.h>           /* for socket programming */
#include "interface.h"          /* for curses interface */

#define TCP_PROTOCOL            6       /* value of tcp in protocol field of ip header of packet */
#define DUMP_BUFFER_SIZE        128     /* size of buffer to pass data to curses window in */
#define SNIFFED_BUFFER_SIZE     65535   /* size of buffer storing sniffed data */

#define SIO_RCVALL  _WSAIOW(IOC_VENDOR, 1)

typedef struct { /* IP Header: 20 mandatory bytes */

/************ { Internet Proctocol Packet Header } **************

 0                   1                   2                   3  
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+      <-----
|Version|  IHL  |Type of Service|          Total Length         |           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+           |
|         Identification        |Flags|     Fragment Offset     |           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+           |
|  Time to Live |    Protocol   |        Header Checksum        |           20 mandatory bytes
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+           |
|                         Source Address                        |           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+           |
|                      Destination Address                      |           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+       <----
|                    Options                    |    Padding    |       Optional fields
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

******************************************************************/

    unsigned char     ip_header_len       : 4;    /* 4-bit header length (for words size of 32-bit) */
    unsigned char     ip_version          : 4;    /* 4-bit IPv4 version */
    unsigned char     ip_tos;                     /* 8-bit IP type of service */
    unsigned short    ip_total_length;            /* 16-bit total IP datagram size */
    
    unsigned short    ip_id;                      /* 16-bit unique identifier */
    unsigned char     ip_frag_offset      : 5;    /* 5 / 13 bits of fragment offset field */
    unsigned char     ip_more_fragment    : 1;    /* 1st flag bit */
    unsigned char     ip_dont_fragment    : 1;    /* 2nd flag bit */
    unsigned char     ip_reserved_zero    : 1;    /* 3rd flag bit */
    unsigned char     ip_frag_offset1;            /* 8 / 13 bits of fragment offset field */

    unsigned char     ip_ttl;                     /* 8-bit time to live */
    unsigned char     ip_protocol;                /* 8-bit protocol: tcp, udp e.t.c. */
    unsigned short    ip_checksum;                /* 16-bit IP Header checksum */

    unsigned int      ip_srcaddr;                 /* 32-bit source address */
    
    unsigned int      ip_destaddr;                /* 32-bit destination address */
 } ip_hdr;

 typedef struct { /* TCP Header */

/******** { Transmission Control Protocol Packet Header} ********

 0                   1                   2                   3  
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          Source Port          |        Destination Port       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Sequence Number                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Acknowledgment Number                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Offset|  Res. |     Flags     |             Window            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|            Checksum           |         Urgent Pointer        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Options                    |    Padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

******************************************************************/

    unsigned short    source_port;                /* 16-bit source port number */
    unsigned short    dest_port;                  /* 16-bit destination port number */
    
    unsigned int      sequence;                   /* 32-bit sequence number */
    
    unsigned int      acknowledge;                /* 32-bit acknowledgement number */

    unsigned char     ns              : 1;        /* 1-bit nonce sum flag */
    unsigned char     reserved_part   : 3;        /* 3-bits of reserved data */
    unsigned char     data_offset     : 4;        /* 4-bit value: stores total size of tcp header in multiple of 4 bytes */
    unsigned char     fin             : 1;        /* 1-bit finish flag */
    unsigned char     syn             : 1;        /* 1-bit synchronize flag */
    unsigned char     rst             : 1;        /* 1-bit reset flag */
    unsigned char     psh             : 1;        /* 1-bit push flag */
    unsigned char     ack             : 1;        /* 1-bit acknowledge flag */
    unsigned char     urg             : 1;        /* 1-bit urgent flag */
    unsigned char     ecn             : 1;        /* 1-bit ecn-echo flag, ecn: explicit congestion notification */
    unsigned char     cwr             : 1;        /* 1-bit congestion window reduced flag */
    unsigned short    window;                     /* 16-bit window size value */

    unsigned short    checksum;                   /* 16-bit tcp header checksum */
    unsigned short    urgent_pointer;             /* 16-bit urgent pointer field */
}tcp_hdr;


DWORD
WINAPI init_sniffer_thread(LPVOID lpParam);

int
init_sniffer(char *address);

void
start_sniffing(SOCKET sniffer, char *address);

void
process_packet(char *address, char *buffer, unsigned int size);

void
print_ip_header(char *buffer);

void
print_tcp_packet(char *buffer, unsigned int size);

void
print_data(char *buffer, unsigned int size);

#endif /* TCP_SNIFFER_H */
