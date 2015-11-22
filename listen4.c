//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#include "listen4.h"
#include "main.h"
#include "send6.h"

int ipv4_listen_s_fd;

/**
 * Closes file descriptors
 */
void listen4_exit() {
    if (close(ipv4_listen_s_fd) == -1) {
        perror("ERROR: listen4_exit(): Failed to close a socket");
        exit(5);
    }
}

/**
 * Initializes socket and it's options
 */
void listen4_init() {
    // Create socket to receive all IPv4 traffic
    ipv4_listen_s_fd = socket(AF_INET, SOCK_RAW, IPPROTO_IPV6);
    if (ipv4_listen_s_fd < 0) {
        perror("ERROR: listen4_init(): Failed to create a socket");
        exit(5);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, args.wan, sizeof(ifr.ifr_name));
    if (setsockopt(ipv4_listen_s_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *) &ifr, sizeof(ifr)) < 0) {
        perror("ERROR: listen4_init(): Failed to SO_BINDTODEVICE");
        exit(5);
    }
}

/**
 * Starts listening on args.wan
 */
void listen4_start() {
    struct sockaddr sin;
    socklen_t sin_l = sizeof(sin);

    ssize_t data_size;
    // MTU + IPv4 header size
    char ipv4_ipv6_buffer[MTU_SIZE + 20];
    char *ipv6_buffer = ipv4_ipv6_buffer + 20;

    while (1) {
        if ((data_size = recvfrom(ipv4_listen_s_fd, ipv4_ipv6_buffer, MTU_SIZE + 20, 0, &sin, &sin_l)) ==
            -1) {
            perror("ERROR: listen4_start(): Failed to recive socket");
            exit(5);
        }

        struct iphdr *hdr = (struct iphdr *) ipv4_ipv6_buffer;
        char tmp_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &hdr->saddr, tmp_str, INET_ADDRSTRLEN);

        // We only want packets from tunnel's end, nothing else
        if (strcmp(args.remote, tmp_str) != 0) {
#ifdef DEBUG
            printf("NOT OURS PACKET :: skipping\n");
#endif
            continue;
        }

#ifdef DEBUG
        char tmp_str[INET_ADDRSTRLEN];
        printf("< DEBUG: listen4_start() >\n");
        inet_ntop(AF_INET, &hdr->saddr, tmp_str, INET_ADDRSTRLEN);
        printf("\tSrc: %s\n", tmp_str);
        inet_ntop(AF_INET, &hdr->daddr, tmp_str, INET_ADDRSTRLEN);
        printf("\tDst: %s\n", tmp_str);
        printf("\tReceived (IPv4 + IPv6): %zu\n", data_size);

        char tmp_str2[INET6_ADDRSTRLEN];
        struct ip6_hdr *hdr_6 = (struct ip6_hdr *) ipv6_buffer;
        struct in6_addr *src = &hdr_6->ip6_src;
        struct in6_addr *dst = &hdr_6->ip6_dst;
        inet_ntop(AF_INET6, src, tmp_str2, INET6_ADDRSTRLEN);
        printf("\tIPv6 Src: %s\n", tmp_str2);
        inet_ntop(AF_INET6, dst, tmp_str2, INET6_ADDRSTRLEN);
        printf("\tIPv6 Dst: %s\n", tmp_str2);

        printf("</ DEBUG >\n");
#endif

        // Send IPv6 packet - IPv4 header
        send6_start(ipv6_buffer, (size_t) data_size - 20);
    }
}