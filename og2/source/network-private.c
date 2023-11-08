#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sdmessage.pb-c.h"
#include "network_server.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

uint8_t *serialize_message(MessageT *message) {
    if (message == NULL) {
        return NULL; 
    }

    size_t message_size = message_t__get_packed_size(message);
    
    if (message_size == 0) {
        return NULL;
    }

    uint8_t *message_data = (uint8_t *)malloc(message_size);
    if (message_data == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    message_t__pack(message, message_data);
    return message_data;
}

MessageT *deserialize_message(uint8_t *message_data, size_t message_size) {
    if (message_data == NULL || message_size == 0) {
        return NULL;
    }

    MessageT *msg = message_t__unpack(NULL, message_size, message_data);
    if (msg == NULL) {
        perror("Message deserialization error");
        return NULL;
    }

    return msg;
}
