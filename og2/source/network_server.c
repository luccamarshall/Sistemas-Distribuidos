#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <unistd.h>

#include "network_server.h"
#include "network-private.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"

#define MAX_BUFFER_SIZE 1024
/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

int network_server_init(short port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        return -1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 1) < 0) {
        perror("Error listening on server socket");
        close(server_socket);
        return -1;
    }

    return server_socket;
}

int network_send(int client_socket, MessageT *msg) {
    // Serialize the message
    size_t message_size = message_t__get_packed_size(msg);
    uint8_t *message_data = serialize_message(msg);

    if (message_data == NULL) {
        return -1;  // Serialization error
    }

    // Send the message size (2 bytes) first
    uint16_t message_size_n = htons((uint16_t)message_size);
    if (write_all(client_socket, &message_size_n, sizeof(uint16_t)) < 0) {
        perror("Error sending message size");
        free(message_data);
        return -1;
    }

    // Send the serialized message
    if (write_all(client_socket, message_data, message_size) < 0) {
        perror("Error sending message data");
        free(message_data);
        return -1;
    }

    free(message_data);
    return 0;
}

MessageT *network_receive(int client_socket) {
    // Read the message size (2 bytes)
    uint16_t message_size_n;
    if (read_all(client_socket, &message_size_n, sizeof(uint16_t)) < 0) {
        perror("Error receiving message size");
        return NULL;
    }

    size_t message_size = ntohs(message_size_n);

    // Read the serialized message data
    uint8_t *message_data = (uint8_t *)malloc(message_size);
    if (message_data == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    if (read_all(client_socket, message_data, message_size) < 0) {
        perror("Error receiving message data");
        free(message_data);
        return NULL;
    }

    // Deserialize the message
    MessageT *msg = deserialize_message(message_data, message_size);

    if (msg == NULL) {
        perror("Deserialization error");
    }

    free(message_data);
    return msg;
}

int network_server_close(int server_socket) {
    close(server_socket);
    return 0;
}

int network_main_loop(int listening_socket, struct table_t *table) {
    
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_socket = accept(listening_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket < 0) {
        perror("Error accepting client connection");
        return -1;
    }

    printf("Client connected\n");

    while (1) {
        MessageT *msg = network_receive(client_socket);
        if (msg == NULL) {
            perror("Error receiving message");
            return -1;
        }

        int result = invoke(msg, table); 

        MessageT response = MESSAGE_T__INIT;

        if (result) {
            // Handle the error case
            response.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            response.c_type = MESSAGE_T__C_TYPE__CT_NONE;
        } else {
            // Handle the success case
            response.opcode = msg->opcode;
            response.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            response.result = result;
        }

        if (network_send(client_socket, &response) < 0) {
            perror("Error sending message");
            return -1;
        }

        message_t__free_unpacked(msg, NULL);
    }

    return 0;
}

