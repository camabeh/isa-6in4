//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#ifndef ISA_6IN4_V2_LISTEN6_H
#define ISA_6IN4_V2_LISTEN6_H

#include <string.h>
#include <netinet/ip6.h>
#include <linux/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>

void listen6_init();
void listen6_start();
void listen6_exit();

#endif //ISA_6IN4_V2_LISTEN6_H
