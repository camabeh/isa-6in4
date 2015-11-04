#ifndef ISA_6IN4_SIXTUNNEL_H
#define ISA_6IN4_SIXTUNNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <error.h>
#include <stdlib.h>
#include <sys/types.h>


#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <pthread.h>
#include <getopt.h>

#define MTU_SIZE 1480

typedef struct {
    char lan[16];
    char wan[16];
    char remote[20];
    char log[20];
} args_t;

void print_usage();

args_t parse_args(int argc, char **argv);

void ipv4_wrap(int ipv4_recive_s_fd, char *buffer, size_t size);
void ipv6_listen(int ipv6_listen_s_fd, int ipv4_recive_s_fd);

#endif //ISA_6IN4_SIXTUNNEL_H
