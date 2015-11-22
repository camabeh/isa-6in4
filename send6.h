//
// Created by root on 21/11/15.
//

#ifndef ISA_6IN4_V2_SEND6_H
#define ISA_6IN4_V2_SEND6_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <unistd.h>

void send6_init();
void send6_exit();
void send6_start(char* buffer, size_t len);

#endif //ISA_6IN4_V2_SEND6_H
