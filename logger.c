//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#include "logger.h"
#include "main.h"

// Flags used for eliminating multiple logging information
int tcp_log = 1;
int imcp_log = 0;

pthread_mutex_t stdout_lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Takes IPv6 packet and prints values
 */
void logger_ipv6(char *buffer) {
    // Get data from IPv6 header structure
    struct ip6_hdr *hdr = (struct ip6_hdr *) buffer;
    struct in6_addr *src = &hdr->ip6_src;
    struct in6_addr *dst = &hdr->ip6_dst;
    uint8_t proto = hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;

    char src_string[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, src, src_string, INET6_ADDRSTRLEN);
    char dst_string[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, dst, dst_string, INET6_ADDRSTRLEN);

    char *protocols[3] = {"icmp", "tcp", "udp"};

    // We only want to print some protocols
    switch (proto) {
        case IPPROTO_ICMPV6: {
            if (imcp_log == 0) {
                logger(protocols[0], src_string, dst_string, 0, 0);
                imcp_log = 1;
            } else {
                imcp_log = 0;
            }
            break;
        }
        case IPPROTO_TCP: {
            struct tcphdr *tcp = (struct tcphdr *) (buffer + sizeof(struct ip6_hdr));
            // Get only valid TCP packets
            if (ntohs(tcp->fin) != 0) {
                if (tcp_log == 0) {
                    logger(protocols[1], src_string, dst_string, ntohs(tcp->th_sport), ntohs(tcp->th_dport));
                    tcp_log = 1;
                } else {
                    tcp_log = 0;
                }
            }
            break;
        }
        case IPPROTO_UDP: {
            struct udphdr *udp = (struct udphdr *) (buffer + sizeof(struct ip6_hdr));
            logger(protocols[2], src_string, dst_string, ntohs(udp->uh_sport), ntohs(udp->uh_dport));
            break;
        }
        default: {

        }
    }
}

/**
 * Core logging function
 * This function takes care of locking and unlocking mutex, because we have 2 threads accessing same resources
 */
void logger(const char *protocol, const char *src, const char *dst, unsigned int src_port, unsigned int dst_port) {
    // Receive file/stdout lock
    pthread_mutex_lock(&stdout_lock);

    time_t now;
    struct tm ts;
    char buf[80];
    // Get current time
    time(&now);
    ts = *localtime(&now);
    // Format time 2015/11/03 20:37:55
    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &ts);

    char *out;

    // Covert port number to string
    char src_port_s[10];
    char dst_port_s[10];
    if (src_port != -1 && dst_port != -1) {
        sprintf(src_port_s, "%u", src_port);
        sprintf(dst_port_s, "%u", dst_port);
    }

    // Concat all data
    asprintf(&out, "%s %s %s %s %s %s -\n", buf, src, dst, protocol,
             (src_port == 0) ? "-": src_port_s,
             (dst_port == 0) ? "-": dst_port_s);

    // Print out to console
    fprintf(stdout, "%s", out);
    fflush(stdout);

    // Print out to log file
    fwrite(out, strlen(out), 1, log_file);
    fflush(log_file);

    // Release file/stdout lock
    pthread_mutex_unlock(&stdout_lock);
}

/**
 *
 */
void open_log(char *name) {
    log_file = fopen(name, "wt");
    if (log_file == NULL) {
        fprintf(stderr, "ERROR: Unable to create log file\n");
        exit(2);
    }
}

void close_log() {
    if (fclose(log_file) == EOF) {
        fprintf(stderr, "ERROR: Unable to close log file\n");
        exit(2);
    }
}