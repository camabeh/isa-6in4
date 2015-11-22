//
// Created by camabeh on 11/21/15.
//

#ifndef ISA_6IN4_V2_LISTEN4_H
#define ISA_6IN4_V2_LISTEN4_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/ip6.h>

#include <unistd.h>


void listen4_init();
void listen4_exit();
void listen4_start();

#endif //ISA_6IN4_V2_LISTEN4_H
