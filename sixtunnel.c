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

#define MTU_SIZE 1280

#define DEBUG 1
typedef struct shared_s {
    int mutex; // 0 open, 1 closed
    int c;
} shared_t;


FILE *log_file;
pthread_mutex_t log_lock;
pthread_mutex_t stdout_lock = PTHREAD_MUTEX_INITIALIZER;

void ipv4_wrap(char *buffer, size_t size);

typedef struct {
    char lan[16];
    char wan[16];
    char remote[20];
    char log[20];
} args_t1;

typedef struct {
    char lan[16];
    char wan[16];
    char remote[20];
    char log[20];
} args_t2;

args_t1 t1 = {
        "eth0",
        "eth1",
        "192.168.0.11",
        "out.log"
};
args_t2 t2;

void parse_args(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "ERROR: Invalid runtime options\n");
        exit(1);
    }
}


void logger_stdout(const char *msg) {
    pthread_mutex_lock(&stdout_lock);
    printf("%s\n", msg);
    pthread_mutex_unlock(&stdout_lock);
}

void logger(char *msg) {
    pthread_mutex_lock(&log_lock);

    fwrite("TEE", strlen("TEE"), 1, log_file);

    pthread_mutex_unlock(&log_lock);
}

void ipv6_listen() {
    int socket_fd;
    char buffer[MTU_SIZE + ETH_HLEN];
//    struct sockaddr_in6 sin6;
    struct sockaddr sin;

    socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IPV6));
    if (socket_fd < 0) {
        perror("ERROR: ipv6_listen: Failed to create a socket");
        exit(3);
    }

//    int a = setsockopt(socket_fd , SOL_SOCKET , SO_BINDTODEVICE , "lo" , strlen("lo")+ 1 );
    struct sockaddr_ll sock_address;
    memset(&sock_address, 0, sizeof(sock_address));
    sock_address.sll_family = PF_PACKET;
    sock_address.sll_protocol = htons(ETH_P_IPV6);
    sock_address.sll_ifindex = if_nametoindex(t1.lan);
    if (bind(socket_fd, (struct sockaddr *) &sock_address, sizeof(sock_address)) < 0) {
        perror("ERROR: ipv6_listen: Failed to bind a socket");
        exit(4);
    }


    ssize_t data_size;
    data_size = recvfrom(socket_fd, buffer, MTU_SIZE + ETH_HLEN, 0, &sin, (socklen_t *) &sin);

    struct ip6_hdr *hdr = (struct ip6_hdr *) (buffer + ETH_HLEN);
    struct in6_addr *src = &hdr->ip6_src;
    struct in6_addr *dst = &hdr->ip6_dst;
    uint32_t flow_id = ntohl(hdr->ip6_ctlun.ip6_un1.ip6_un1_flow);
    uint16_t payload_length = ntohs(hdr->ip6_ctlun.ip6_un1.ip6_un1_plen);
    uint8_t next_header = hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;
    uint8_t hop_limit = hdr->ip6_ctlun.ip6_un1.ip6_un1_hlim;
    uint8_t version = hdr->ip6_ctlun.ip6_un2_vfc;


#if DEBUG
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


    ipv4_wrap(buffer + ETH_HLEN, payload_length + sizeof(struct ip6_hdr));
//    ipv4_wrap("", 0);
}

void ipv4_wrap(char *buffer, size_t size) {

    // Create File descrptor for socket
    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_IPV6)) < 0) {
        perror("ERROR: ipv4_wrap: Failed to create a socket");
        exit(3);
    }

    // I will supply IP Header
    int hdrincl = 1;
    if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl)) == -1) {
        perror("ERROR: ipv4_wrap: Failed to bind a socket");
        exit(4);
    }

    char *srcIP = "192.168.0.10";
    char *dstIP = t1.remote;

    // Create address structure
    struct sockaddr_in daddr;
    daddr.sin_family = AF_INET;
    daddr.sin_port = 0; // Raw sockets can't use ports
    inet_pton(AF_INET, srcIP, &daddr.sin_addr);
    memset(daddr.sin_zero, 0, sizeof(daddr.sin_zero));

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
    ip_header->check = htons(0);  //16 bit checksum of IP header. Can't calculate at this point
    ip_header->saddr = inet_addr(srcIP);
    ip_header->daddr = inet_addr(dstIP);

    memcpy(data, buffer, size);

//    ip_header->check = csum((unsigned short *) packet, ip_header->tot_len);
#if DEBUG
    char str[INET_ADDRSTRLEN];
    printf("\n----\t----\tSTART\t----\t----\n");
    printf("IPv4 tot_len: \t\t\t\t\t %u\n", sizeof(packet));
//    inet_ntop(AF_INET, inet_addr(srcIP), str, INET_ADDRSTRLEN);
//    printf("IPv4 source: %s\n", str);
//    inet_ntop(AF_INET, (struct in_addr *) ip_header->daddr, str, INET_ADDRSTRLEN);
//    printf("IPv4 dest: %s\n", str);
    printf("Payload: %3s\n", data + sizeof(struct ip6_hdr));
    printf("----\t----\t END \t----\t----\n");
#endif

//    while (1) {
//        sleep(1);
    ssize_t r;
    if ((r = sendto(socket_fd, (char *) packet, sizeof(packet), 0,
               (struct sockaddr *) &daddr, (socklen_t) sizeof(daddr))) < 0) {
        perror("ERROR: ipv4_wrap: Failed to send send packet");
        exit(5);
    }

    printf("%u", r);
//    }
}


pthread_t t_id[2];
void *tunnel_4in6(void *arg) {
#ifdef DEBUG
    logger_stdout("Spawned tunnel_4in6 thread");
#endif

    return NULL;
}

void *tunnel_6in4(void *arg) {
#ifdef DEBUG
    logger_stdout("Spawned tunnel_6in4 thread");
#endif

    return NULL;
}

int main(int argc, char **argv) {
    log_file = fopen(t1.log, "wt");

//    t1.lan = "lo";
//    t1.lan = "lo";

//    parse_args(argc, argv);

//    ipv6_listen();
//    ipv4_wrap("", 0);

    int s1 = pthread_create(&(t_id[0]), NULL, &tunnel_4in6, NULL);
    int s2 = pthread_create(&(t_id[1]), NULL, &tunnel_6in4, NULL);

    if (s1 || s2) {
        perror("ERROR: Create thread");
        exit(2);
    }

    pthread_join(t_id[0], NULL);
    pthread_join(t_id[1], NULL);

    return 0;
}
