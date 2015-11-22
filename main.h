//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#ifndef ISA_6IN4_V2_MAIN_H
#define ISA_6IN4_V2_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <bits/sigthread.h>

#define MTU_SIZE 1480
//#define DEBUG 0

typedef struct {
    char lan[16];
    char wan[16];
    char remote[20];
    char log[20];
} args_t;

args_t args;
pthread_t t_id[2]; // Threads
FILE *log_file;

void print_usage();

void *tunnel_6in4(void *arg);

void *tunnel_4in6(void *arg);



void t6in4_int_handler(int arg);

void t4in6_int_handler(int arg);

void main_int_handler(int arg);

#endif //ISA_6IN4_V2_MAIN_H
