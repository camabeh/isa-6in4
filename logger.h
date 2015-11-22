//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#ifndef ISA_6IN4_V2_LOGGER_H
#define ISA_6IN4_V2_LOGGER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>

void logger_ipv6(char *packet);

void logger(const char *protocol, const char *src, const char *dst, unsigned int src_port, unsigned int dst_port);

void open_log(char *name);

void close_log();

#endif //ISA_6IN4_V2_LOGGER_H
