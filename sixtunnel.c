#include "sixtunnel.h"
#define DEBUG 1

args_t args;
FILE *log_file;
pthread_mutex_t log_lock;
pthread_mutex_t stdout_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t t_id[2];

/**
 * Prints usage
 */
void print_usage() {
    printf("./sixtunnel --lan eth0 --wan eth1 --remote 192.168.0.11 --log output.log\n");
}

/**
 * Parses arguments, verifies them and returns args_t structure
 */
args_t parse_args(int argc, char **argv) {
    args_t args;

    // TODO check for error values

    if (argc != 9) {
        fprintf(stderr, "ERROR: Invalid runtime options\n");
        exit(EXIT_FAILURE);
    };

    static struct option long_options[] = {
            {"lan",    required_argument, 0, 'l'}, // eth0
            {"wan",    required_argument, 0, 'w'}, // eth1
            {"remote", required_argument, 0, 'r'}, // 192.168.0.11
            {"log",    required_argument, 0, 'i'}, // output.log
            {0, 0,                        0, 0}
    };

    int opt, long_index = 0;
    while ((opt = getopt_long(argc, argv, "l:w:r:i:",
                              long_options, &long_index)) != -1) {
#ifdef DEBUG
        printf("%d %s\n", optind, optarg);
#endif
        switch (opt) {
            case 'l' :
                strncpy(args.lan, optarg, 16);
                break;
            case 'w' :
                strncpy(args.wan, optarg, 16);
                break;
            case 'r' :
                strncpy(args.remote, optarg, 20);
                break;
            case 'i' :
                strncpy(args.log, optarg, 20);
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    return args;
}

/**
 * Logs msg with time to the file and stdout, including \n
 */
void logger(const char *msg) {
    pthread_mutex_lock(&stdout_lock);

    time_t now;
    struct tm ts;
    char buf[80];

    // Get current time
    time(&now);
    ts = *localtime(&now);

    // Format time 2015/11/03 20:37:55
    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &ts);

    char log_record[256];
    size_t offset = strlen(buf);
    size_t log_record_size = offset + 1 + strlen(msg);

    strcpy(log_record, buf);
    log_record[offset] = ' ';
    strcpy(log_record + offset + 1, msg);
    log_record[log_record_size] = '\n';
    log_record[log_record_size + 1] = '\0';

    // Print out to console
    printf("%s", log_record);

    // Print out to log file
    fwrite(log_record, log_record_size + 1, 1, log_file);
    fflush(log_file);

    pthread_mutex_unlock(&stdout_lock);
}


void ipv6_listen(int ipv6_listen_s_fd, int ipv4_recive_s_fd) {
    char buffer[MTU_SIZE + ETH_HLEN];
//    struct sockaddr_in6 sin6;
    struct sockaddr sin;

    while (1) {
        ssize_t data_size;
        data_size = recvfrom(ipv6_listen_s_fd, buffer, MTU_SIZE + ETH_HLEN, 0, &sin, (socklen_t *) &sin);

        struct ip6_hdr *hdr = (struct ip6_hdr *) (buffer + ETH_HLEN);
        struct in6_addr *src = &hdr->ip6_src;
        struct in6_addr *dst = &hdr->ip6_dst;
        uint32_t flow_id = ntohl(hdr->ip6_ctlun.ip6_un1.ip6_un1_flow);
        uint16_t payload_length = ntohs(hdr->ip6_ctlun.ip6_un1.ip6_un1_plen);
        uint8_t next_header = hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        uint8_t hop_limit = hdr->ip6_ctlun.ip6_un1.ip6_un1_hlim;
        uint8_t version = hdr->ip6_ctlun.ip6_un2_vfc;


#ifndef DEBUG
        char str[INET6_ADDRSTRLEN];
        printf("\n----\t----\tSTART\t----\t----\n");
        printf("Received data_size: \t\t\t %u\n", data_size - ETH_HLEN);
        printf("IPv6 flow_id: \t\t\t\t\t %u\n", flow_id);
        printf("Payload length + IPv6 HDR: \t\t %u\n", payload_length + sizeof(struct ip6_hdr));
        printf("IPv6 payload_length: \t\t\t %u\n", payload_length);
        printf("IPv6 next_header(0 = None): \t %u\n", (next_header == IPPROTO_NONE) ? 0 : next_header);
        printf("IPv6 hop_limit: \t\t\t\t %u\n", hop_limit);
        printf("IPv6 version: \t\t\t\t\t %u\n", version);
        inet_ntop(AF_INET6, src, str, INET6_ADDRSTRLEN);
        printf("IPv6 source: %s\n", str);
        inet_ntop(AF_INET6, dst, str, INET6_ADDRSTRLEN);
        printf("IPv6 dest: %s\n", str);
        printf("Payload: %3s\n", buffer + ETH_HLEN + sizeof(struct ip6_hdr));
        printf("----\t----\t END \t----\t----\n");
#endif

        // Wrap current socket without ethernet header and send it
        ipv4_wrap(ipv4_recive_s_fd, buffer + ETH_HLEN, payload_length + sizeof(struct ip6_hdr));
    }

}

/**
 * IPv4 filed descriptor, IPv6 socket in buffer, size of IPv6 socket
 */
void ipv4_wrap(int ipv4_send_s_fd, char *buffer, size_t size) {

    char *srcIP = "192.168.0.10";
    char *dstIP = args.remote;

#ifdef DEBUG
    printf("ipv4_wrap() src: %s\n", srcIP);
    printf("ipv4_wrap() dst: %s\n", dstIP);
    printf("ipv4_wrap() size of IPv6 socket: %lu\n", size);
#endif

    // Create address structure
    struct sockaddr_in din;
    din.sin_family = AF_INET;
    din.sin_port = 0; // Raw sockets can't use ports
    inet_pton(AF_INET, dstIP, &din.sin_addr);
//    memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

    // Create a Packet
    char packet[size + sizeof(struct iphdr)]; // TODO
    memset(packet, 0, sizeof(packet));

    // Structure packet
    struct iphdr *ip_header = (struct iphdr *) packet;
    // Data to be appended at the end of the ip header
    char *data = (char *) (packet + (sizeof(struct iphdr)));
    ip_header->version = 4; // IPv4
    ip_header->ihl = 5; // 5 x 32-bit words
    ip_header->tos = 0; // Type of service
//    ip_header->tot_len = htons(sizeof(struct iphdr) + strlen(data)); // Total IP packet length
    ip_header->tot_len = htons(sizeof(packet)); // Total IP packet length
    ip_header->protocol = IPPROTO_IPV6; // 6in4 protocol
    ip_header->frag_off = 0x00; //16 bit field = [0:2] flags + [3:15] offset = 0x0
    ip_header->ttl = 0xFF; // Max number of hops 16bit
//    ip_header->id = htons(54321); // 0x00; //16 bit id
//    ip_header->check = htons(0);  //16 bit checksum of IP header. Can't calculate at this point
    ip_header->saddr = inet_addr(srcIP);
    ip_header->daddr = inet_addr(dstIP);

    memcpy(data, buffer, size);

//    ip_header->check = csum((unsigned short *) packet, ip_header->tot_len);
#ifndef DEBUG
    char str[INET_ADDRSTRLEN];
    printf("\n----\t----\tSTART\t----\t----\n");
    printf("IPv4 tot_len: \t\t\t\t\t %u\n", sizeof(packet));
//    inet_ntop(AF_INET, inet_addr(srcIP), str, INET_ADDRSTRLEN);
//    printf("IPv4 source: %s\n", str);
//    inet_ntop(AF_INET, (struct in_addr *) ip_header->sin, str, INET_ADDRSTRLEN);
//    printf("IPv4 dest: %s\n", str);
    printf("Payload: %3s\n", data + sizeof(struct ip6_hdr));
    printf("----\t----\t END \t----\t----\n");
#endif

//        sleep(1);
    ssize_t r;
    if ((r = sendto(ipv4_send_s_fd, (char *) packet, sizeof(packet), 0,
                    (struct sockaddr *) &din, (socklen_t) sizeof(din))) < 0) {
        perror("ERROR: ipv4_wrap: Failed to send send packet");
        exit(7);
    }

    printf("%u\n", r);
}


/**
 * Creates IPv6 to IPv4 tunnel, listens on --lan, wraps IPv6 packet to IPv4
 * socket and forwards to --remote
 */
void *tunnel_6in4(void *arg) {
#ifdef DEBUG
    logger("Spawned tunnel_6in4 thread");
#endif

    int ipv6_listen_s_fd;
    int ipv4_send_s_fd; // IPv4 socket fd to avoid creating new socket for every request

    // Create socket to receive all IPv6 traffic
    ipv6_listen_s_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IPV6));
    if (ipv6_listen_s_fd < 0) {
        perror("ERROR: ipv6_listen: Failed to create a socket");
        exit(3);
    }

    // Bind socket to receive all IPv6 traffic
    struct sockaddr_ll sock_address;
    memset(&sock_address, 0, sizeof(sock_address));
    sock_address.sll_family = PF_PACKET;
    sock_address.sll_protocol = htons(ETH_P_IPV6);
    sock_address.sll_ifindex = if_nametoindex(args.lan);
    if (bind(ipv6_listen_s_fd, (struct sockaddr *) &sock_address, sizeof(sock_address)) < 0) {
        perror("ERROR: ipv6_listen: Failed to bind a socket");
        exit(4);
    }

    // Create socket to be able send IPv4 packets
    if ((ipv4_send_s_fd = socket(AF_INET, SOCK_RAW, IPPROTO_IPV6)) < 0) {
        perror("ERROR: ipv4_wrap: Failed to create a socket");
        exit(5);
    }

    // I will supply IP Header
    int hdrincl = 1;
    if (setsockopt(ipv4_send_s_fd, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl)) == -1) {
        perror("ERROR: ipv4_wrap: Failed to bind a socket");
        exit(6);
    }

    // Start listening for IPv6 traffic
    ipv6_listen(ipv6_listen_s_fd, ipv4_send_s_fd);

#ifdef DEBUG
    logger("Exiting tunnel_6in4 thread");
#endif
    return NULL;
}


/**
 * Creates IPv4 to IPv6 tunnel, listens on this IPv4 address and unpacks
 * packet and forwards it (all info in IPv6 header)
 */
void *tunnel_4in6(void *arg) {
#ifdef DEBUG
    logger("Spawned tunnel_4in6 thread");
#endif



#ifdef DEBUG
    logger("Exiting tunnel_4in6 thread");
#endif
    return NULL;
}


int main(int argc, char **argv) {

    // Fill args_t global structure
    args = parse_args(argc, argv);

    // Create file for logging
    log_file = fopen(args.log, "wt");
    if (log_file == NULL) {
        fprintf(stderr, "ERROR: Unable to create log file\n");
        exit(EXIT_FAILURE);
    }

    // Create threads for 2 tunnels, one for sending, one for receiving
    int s1 = pthread_create(&(t_id[0]), NULL, &tunnel_4in6, NULL);
    int s2 = pthread_create(&(t_id[1]), NULL, &tunnel_6in4, NULL);
    if (s1 || s2) {
        perror("ERROR: Create thread");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish, then continue
    pthread_join(t_id[0], NULL);
    pthread_join(t_id[1], NULL);

    // Close file for logging
    if (fclose(log_file) == EOF) {
        fprintf(stderr, "ERROR: Unable to close log file\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
