//
// Created by Jakub Faber (xfaber02) on 20/11/15.
//

#include "main.h"
#include "listen6.h"
#include "send4.h"
#include "listen4.h"
#include "send6.h"
#include "logger.h"

/**
 * Prints usage
 */
void print_usage() {
    printf("./sixtunnel --lan eth0 --wan eth1 --remote 192.168.0.11 --log output.log\n");
}

/**
 * Parse args
 */
args_t parse_args(int argc, char **argv) {
    args_t args = {0,};

    // TODO check for error values

    if (argc != 9) {
        fprintf(stderr, "ERROR: Invalid runtime options\n");
        print_usage();
        exit(1);
    };

    static struct option long_options[] = {
            {"lan",    required_argument, 0, 'l'}, // eth0
            {"wan",    required_argument, 0, 'w'}, // eth1
            {"remote", required_argument, 0, 'r'}, // 192.168.0.11
            {"log",    required_argument, 0, 'i'}, // output.log
            {0, 0,                        0, 0}
    };

#ifdef DEBUG
    printf("<DEBUG: ");
#endif

    int opt, long_index = 0;
    while ((opt = getopt_long(argc, argv, "l:w:r:i:",
                              long_options, &long_index)) != -1) {
#ifdef DEBUG
        printf("%d %s ,", optind, optarg);
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
                exit(1);
        }
    }
#ifdef DEBUG
    printf(" />\n");
#endif

    return args;
}

/**
 * Thread 6in4
 */
void *tunnel_6in4(void *arg) {
#ifdef DEBUG
    printf("<DEBUG: tunnel_6in4()>\n");
#endif
    signal(SIGINT, t6in4_int_handler);

    send4_init();
    listen6_init();

    listen6_start();

    listen6_exit();
    send4_exit();
#ifdef DEBUG
    printf("</DEBUG: tunnel_6in4()>\n");
#endif
    return NULL;
}

/**
 * Thread 4in6
 */
void *tunnel_4in6(void *arg) {
#ifdef DEBUG
    printf("<DEBUG: tunnel_4in6()>\n");
#endif
    signal(SIGINT, t4in6_int_handler);

    listen4_init();
    send6_init();

    listen4_start();

    send6_exit();
    listen4_exit();
#ifdef DEBUG
    printf("</DEBUG: tunnel_4in6()>\n");
#endif
    return NULL;
}

/**
 * SIGINT handler for thread 6in4
 */
void t6in4_int_handler(int arg) {
    listen6_exit();
    send4_exit();
    exit(0);
}

/**
 * SIGINT handler for thread 4in6
 */
void t4in6_int_handler(int arg) {
    send6_exit();
    listen4_exit();
    exit(0);
}

/**
 * SIGINT handler for main process
 */
void main_int_handler(int arg) {
    // Send SIGINT to threads, so they can close file descriptors
    pthread_kill(t_id[0], SIGINT);
    pthread_kill(t_id[1], SIGINT);
    close_log();
    exit(0);
}

int main(int argc, char **argv) {
    // Register signal handler
    signal(SIGINT, main_int_handler);

    args = parse_args(argc, argv);

    // Create file for logging
    open_log(args.log);

    // Create threads for listening/sending
    int s1 = pthread_create(&(t_id[0]), NULL, &tunnel_6in4, NULL);
    int s2 = pthread_create(&(t_id[1]), NULL, &tunnel_4in6, NULL);
    if (s1 || s2) {
        perror("ERROR: Create thread");
        exit(2);
    }

    // Wait for threads to finish
    pthread_join(t_id[0], NULL);
    pthread_join(t_id[1], NULL);

    // Close file for logging
    close_log();

    return 0;
}