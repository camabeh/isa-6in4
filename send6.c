//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#include "send6.h"
#include "logger.h"
#include "main.h"

int ipv6_send_s_fd;

/**
 * Closes file descriptors
 */
void send6_exit() {
    if (close(ipv6_send_s_fd) == -1) {
        perror("ERROR: send6_exit(): Failed to close a socket");
        exit(6);
    }
}

/**
 * Initializes file descriptors
 */
void send6_init() {
    if ((ipv6_send_s_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("ERROR: send6_init(): Failed to create a socket");
        exit(6);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, args.lan, sizeof(ifr.ifr_name));
    if (setsockopt(ipv6_send_s_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("ERROR: send6_init(): Failed to SO_BINDTODEVICE");
        exit(6);
    }
}

/**
 * Sends IPv6 packet in {buffer} via bound interface on args.lan
 */
void send6_start(char* buffer, size_t len) {
    struct sockaddr_in6 dst_addr;
    memset (&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin6_family = AF_INET6;
    dst_addr.sin6_port = htons(0);

    // Fill dst_addr, so we can forward packet to the right IP address
    char str2[INET6_ADDRSTRLEN];
    struct ip6_hdr *hdr = (struct ip6_hdr *) buffer;
    struct in6_addr *src = &hdr->ip6_src;
    struct in6_addr *dst = &hdr->ip6_dst;
    inet_ntop(AF_INET6, dst, str2, INET6_ADDRSTRLEN);
    inet_pton(AF_INET6, str2, &dst_addr.sin6_addr);
    socklen_t len2 = sizeof(dst_addr);

#ifdef DEBUG
    printf("< DEBUG: send6_start() >\n");
    char tmp_str[INET6_ADDRSTRLEN];

    int len3 = sizeof(struct sockaddr_in6);
    struct sockaddr_in6 sin;
    getsockname(ipv6_send_s_fd, (struct sockaddr *) &sin, &len3);
    inet_ntop(AF_INET6, &sin.sin6_addr, tmp_str, INET6_ADDRSTRLEN);
    printf("\tThis Src: %s:%d\n", tmp_str, ntohs(sin.sin6_port));
    printf("\tThis Dst: %s\n", str2);
    inet_ntop(AF_INET6, src, tmp_str, INET6_ADDRSTRLEN);
    printf("\tIPv6 Src: %s\n", tmp_str);
    inet_ntop(AF_INET6, dst, tmp_str, INET6_ADDRSTRLEN);
    printf("\tIPv6 Dst: %s\n", tmp_str);
#endif

    // Log to file and stdout
    logger_ipv6(buffer);

    ssize_t data_size = sendto (ipv6_send_s_fd, buffer, len, 0, (struct sockaddr*) &dst_addr, len2);
    if (data_size == -1) {
        perror("ERROR: send6_start(): Error sending packet");
        exit(6);
    }

#ifdef DEBUG
    printf("\tSent: %zu\n", data_size);
    printf("< /DEBUG >\n");
#endif
}
