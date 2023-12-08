#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"

struct rtable_t {
    char *server_address;
    int server_port;
    int sockfd;
    int successor_sockfd;
    char *successor_address;
    int successor_node_id;
};

#endif