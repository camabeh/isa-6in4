//
// Created by root on 20/11/15.
//

#ifndef ISA_6IN4_V2_SEND4_H
#define ISA_6IN4_V2_SEND4_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/if.h>
#include <string.h>
#include <netinet/ip6.h>
#include <unistd.h>
#include <stddef.h>

void send4_init();
void send4_exit();
void send4_start(char *buffer, size_t size);

#endif //ISA_6IN4_V2_SEND4_H
