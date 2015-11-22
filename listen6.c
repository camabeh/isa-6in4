//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#include "listen6.h"
#include "logger.h"
#include "send4.h"
#include "main.h"

int ipv6_listen_s_fd;

/**
 * Closes file descriptors
 */
void listen6_exit() {
    if (close(ipv6_listen_s_fd) == -1) {
        perror("ERROR: listen6_exit(): Failed to close a socket");
        exit(3);
    }
}

/**
 * Initializes socket and it's options
 */
void listen6_init() {
    // Create socket to receive all IPv6 traffic
    ipv6_listen_s_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IPV6));
    //  ipv6_listen_s_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (ipv6_listen_s_fd < 0) {
        perror("ERROR: listen6_init(): Failed to create a socket");
        exit(3);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, args.lan, sizeof(ifr.ifr_name));
    if (setsockopt(ipv6_listen_s_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("ERROR: listen6_init(): Failed to SO_BINDTODEVICE");
        exit(3);
    }
}

/**
 * Starts listening on args.lan
 */
void listen6_start() {
    struct sockaddr sin;
    ssize_t data_size;
    char eth_buffer[MTU_SIZE + ETH_HLEN];
    char *ipv6_buffer = eth_buffer + ETH_HLEN;

    while (1) {
        if ((data_size = recvfrom(ipv6_listen_s_fd, eth_buffer, MTU_SIZE + ETH_HLEN, 0, &sin, (socklen_t *) &sin)) == -1) {
            perror("ERROR: listen6_start(): Failed to recive socket");
            exit(3);
        }

        if (data_size > MTU_SIZE) {
            printf("TOO LARGE PACKET\n");
            // TODO send too much byes; send ICMPv6 packet
            continue;
        }
        struct ip6_hdr *hdr = (struct ip6_hdr *) ipv6_buffer;
        struct in6_addr *src = &hdr->ip6_src;
        struct in6_addr *dst = &hdr->ip6_dst;
        uint8_t next = hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        size_t ipv6_buffer_len = (size_t ) data_size - ETH_HLEN;

        char tmp_str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, dst, tmp_str, INET6_ADDRSTRLEN);
        if (tmp_str[0] == 'f' && tmp_str[1] == 'f' && tmp_str[2] == '0' && tmp_str[3] == '2') {
#ifdef DEBUG
            printf("\tSKIPPING , NEIGHTBOR SOLICITITAION\n");
#endif
            continue;
        } // SKIP NEIGHTBOR SOLICITITAION packets
        if (tmp_str[0] == 'f' && tmp_str[1] == 'e' && tmp_str[2] == '8' && tmp_str[3] == '0') {
            continue;
        } // SKIP Local-Link packets
        inet_ntop(AF_INET6, src, tmp_str, INET6_ADDRSTRLEN);
        if (tmp_str[0] == 'f' && tmp_str[1] == 'e' && tmp_str[2] == '8' && tmp_str[3] == '0') {
            continue;
        } // SKIP Local-Link packets

        // Log to file and stdout
        logger_ipv6(ipv6_buffer);

#ifdef DEBUG
        printf("< DEBUG: listen6_start() >\n");
        inet_ntop(AF_INET6, src, tmp_str, INET6_ADDRSTRLEN);
        printf("\tSrc: %s\n", tmp_str);
        inet_ntop(AF_INET6, dst, tmp_str, INET6_ADDRSTRLEN);
        printf("\tDst: %s\n", tmp_str);
        printf("\tProto(next): %u\n", next);
        printf("\tReceived data size (w/o ETH): %zu\n", ipv6_buffer_len);
        printf("\tReceived data size (inc ETH): %zu\n", data_size);
        printf("</ DEBUG >\n");
#endif
        // Pass ipv6_buffer to let send4_start wrap and send IPv6 packet inside IPv4
        send4_start(ipv6_buffer, ipv6_buffer_len);
    }
}