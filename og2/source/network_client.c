#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "network-private.h"
#include "client_stub-private.h"
#include "message-private.h"
#include "client_stub.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

int network_connect(struct rtable_t *rtable, int port) {
    if (rtable == NULL) {
        fprintf(stderr, "Invalid rtable parameter.\n");
        return -1;
    }

    // Create a socket
    rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rtable->sockfd == -1) {
        perror("Failed to create a socket");
        return -1;
    }

    // Set up the server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(rtable->server_address);

    // Connect to the server
    if (connect(rtable->sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to the server");
        close(rtable->sockfd);
        return -1;
    }

    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if (rtable == NULL || msg == NULL) {
        return NULL;
    }

    // Serializar a mensagem contida em msg
    size_t serialized_size = message_t__get_packed_size(msg);
    uint8_t *serialized_msg = malloc(serialized_size);

    if (serialized_msg == NULL) {
        return NULL;
    }

    message_t__pack(msg, serialized_msg);

    // Enviar a dimensão do buffer
    int serialized_size_short = (int)serialized_size;
    if (write(rtable->sockfd, &serialized_size_short, sizeof(int)) != sizeof(int)) {
        free(serialized_msg);
        return NULL;
    }

    // Enviar o buffer serializado
    if (write(rtable->sockfd, serialized_msg, serialized_size) != (ssize_t)serialized_size) {
        free(serialized_msg);
        return NULL;
    }

    free(serialized_msg);

    // Receber a dimensão da resposta
    int response_size;
    if (read(rtable->sockfd, &response_size, sizeof(int)) != sizeof(int)) {
        return NULL;
    }

    // Receber a resposta
    uint8_t *response_buffer = malloc(response_size);
    if (read(rtable->sockfd, response_buffer, response_size) != (ssize_t)response_size) {
        free(response_buffer);
        return NULL;
    }

    // De-serializar a resposta
    MessageT *response = message_t__unpack(NULL, response_size, response_buffer);
    free(response_buffer);

    return response;
}



int network_close(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return -1;
    }

    if (close(rtable->sockfd) < 0) {
        return -1;
    }

    return 0;
}