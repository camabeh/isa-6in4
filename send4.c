//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//
#include "send4.h"
#include "main.h"

int ipv4_send_s_fd;

/**
 * Closes file descriptors
 */
void send4_exit() {
    if (close(ipv4_send_s_fd) == -1) {
        perror("ERROR: send6_exit(): Failed to close a socket");
        exit(4);
    }
}

/**
 * Initializes file descriptors
 */
void send4_init() {
    // Create socket to be able send IPv4 packets, kernel fills necessary values
    if ((ipv4_send_s_fd = socket(PF_INET, SOCK_RAW, IPPROTO_IPV6)) < 0) {
        perror("ERROR: send4_init(): Failed to create a socket");
        exit(4);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, args.wan, sizeof(ifr.ifr_name));
    if (setsockopt(ipv4_send_s_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("ERROR: send4_init(): Failed to SO_BINDTODEVICE");
        exit(4);
    }
}

/**
 * Sends IPv4 packet to args.remote, takes IPv6 packet
 */
void send4_start(char *buffer, size_t size) {
    char *dstIP = args.remote;

    // Send to args.remote, kernel fills all other values
    struct sockaddr_in din;
    din.sin_family = AF_INET;
    din.sin_port = 0; // Raw sockets can't use ports
    inet_pton(AF_INET, dstIP, &din.sin_addr);

    ssize_t sent_size;
    if ((sent_size = sendto(ipv4_send_s_fd, buffer, size, 0, (struct sockaddr *) &din, (socklen_t) sizeof(din))) < 0) {
        perror("ERROR: send4_start(): Failed to send send packet");
        exit(4);
    }

#ifdef DEBUG
    printf("< DEBUG: send4_start() >\n");

    int len = sizeof(struct sockaddr);
    struct sockaddr_in sin;
    getsockname(ipv4_send_s_fd, (struct sockaddr *) &sin, &len);
    printf("\tSrc: %s:%d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    printf("\tDst: %s\n", args.remote);
    printf("\tSent (IPv6 size only): %zu\n", sent_size);

    char tmp_str[INET6_ADDRSTRLEN];
    struct ip6_hdr *hdr = (struct ip6_hdr *) buffer;
    struct in6_addr *src = &hdr->ip6_src;
    struct in6_addr *dst = &hdr->ip6_dst;
    inet_ntop(AF_INET6, src, tmp_str, INET6_ADDRSTRLEN);
    printf("\tIPv6 Src: %s\n", tmp_str);
    inet_ntop(AF_INET6, dst, tmp_str, INET6_ADDRSTRLEN);
    printf("\tIPv6 Dst: %s\n", tmp_str);

    printf("</ DEBUG >\n");
#endif
}