#include "stdio.h" 
#include <stdlib.h>
#include "client_stub.h"
#include "client_stub-private.h"
#include "message-private.h"

struct rtable_t *rtable_connect(char *address_port) {
  
    if (address_port == NULL) {
        fprintf(stderr, "rtable_connect: Invalid argument\n");
        return NULL;
    }

    char *address = strtok(address_port, ":");
    char *port = strtok(NULL, ":");

    if (address == NULL || port == NULL) {
        fprintf(stderr, "rtable_connect: Invalid argument format (use <hostname>:<port>)\n");
        return NULL;
    }

    // Initialize rtable structure
    struct rtable_t *rtable = (struct rtable_t *)malloc(sizeof(struct rtable_t));
    if (rtable == NULL) {
        perror("rtable_connect: malloc failed");
        return NULL;
    }

    // Initialize server address and port
    rtable->server_address = strdup(address);
    if (rtable->server_address == NULL) {
        perror("rtable_connect: strdup(server_address) failed");
        free(rtable);
        return NULL;
    }
    rtable->server_port = atoi(port);

    // Create a socket and establish a connection
    int sockfd = network_connect(address, rtable->server_port);
    if (sockfd < 0) {
        perror("rtable_connect: network_connect failed");
        free(rtable->server_address);
        free(rtable);
        return NULL;
    }

    rtable->sockfd = sockfd;

    return rtable;
}


int rtable_disconnect(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return -1;
    }

    if(network_close(rtable->sockfd) < 0) {
        perror("rtable_disconnect: network_close failed");
        return -1;
    }

    free(rtable->server_address);
    if (close(rtable->sockfd) < 0) {
        perror("rtable_disconnect: close socket failed");
        return -1;
    }

    free(rtable);
    return 0;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    if (rtable == NULL || entry == NULL) {
        return -1;
    }

    // MessageT *request = create_put_request(entry);
    MessageT *request = MESSAGE_T__INIT;
    request->opcode = MESSAGE_T__OPCODE__OP_PUT;
    request->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    request->key = entry->key;

    EntryT *entry_message = ENTRY_T__INIT;
    entry_message->key = entry->key;
    entry_message->value.data = (uint8_t *) entry->value->data;
    entry_message->value.len = (size_t) entry->value->datasize;

    request->content.entry = entry_message;
    
    // Send the message and receive the response
    MessageT *response = network_send_receive(rtable->socket, request);

    // Check if the response is valid
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_put: response not valid\n");
        return -1;
    }
    free(request);
    free(response);
    free(entry_message);

    return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL) {
        return NULL;
    }

    // Send request type
    int request_type = GET;
    if (write_all(rtable->sockfd, &request_type, sizeof(int)) < 0) {
        perror("rtable_get: write request_type failed");
        return NULL;
    }

    // Send key
    if (write_all(rtable->sockfd, key, strlen(key) + 1) < 0) {
        perror("rtable_get: write key failed");
        return NULL;
    }

    // Receive result
    int result;
    if (read_all(rtable->sockfd, &result, sizeof(int)) < 0) {
        perror("rtable_get: read result failed");
        return NULL;
    }

    if (result == 0) {
        // Receive data
        struct data_t *data = data_unmarshall(rtable->sockfd);
        if (data == NULL) {
            perror("rtable_get: data_unmarshall failed");
            return NULL;
        }

        return data;
    } else {
        return NULL;
    } 
}

int rtable_del(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL) {
        return -1;
    }

    // Send request type
    int request_type = DEL;
    if (write_all(rtable->sockfd, &request_type, sizeof(int)) < 0) {
        perror("rtable_del: write request_type failed");
        return -2;
    }

    // Send key
    size_t key_length = strlen(key) + 1;
    if (write_all(rtable->sockfd, key, key_length) < 0) {
        perror("rtable_del: write key failed");
        return -3;
    }

    // Receive result
    int result;
    if (read_all(rtable->sockfd, &result, sizeof(int)) < 0) {
        perror("rtable_del: read result failed");
        return -4;
    }

    return result;
}

int rtable_size(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return -1;
    }

    // Send request type
    int request_type = SIZE;
    if (write_all(rtable->sockfd, &request_type, sizeof(int)) < 0) {
        perror("rtable_size: write request_type failed");
        return -2;
    }

    // Receive result
    int result;
    if (read_all(rtable->sockfd, &result, sizeof(int)) < 0) {
        perror("rtable_size: read result failed");
        return -3;
    }

    return result;
}

char **rtable_get_keys(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return NULL;
    }

    // Send request type
    int request_type = GETKEYS;
    if (write_all(rtable->sockfd, &request_type, sizeof(int)) < 0) {
        perror("rtable_get_keys: write request_type failed");
        return NULL;
    }

    // Receive result
    int result;
    if (read_all(rtable->sockfd, &result, sizeof(int)) < 0) {
        perror("rtable_get_keys: read result failed");
        return NULL;
    }

    if (result == 0) {
        // Receive keys
        char **keys = keys_unmarshall(rtable->sockfd);
        if (keys == NULL) {
            perror("rtable_get_keys: keys_unmarshall failed");
            return NULL;
        }

        return keys;
    } else {
        return NULL;
    }
}

void rtable_free_keys(char **keys) {
    if (keys == NULL) {
        return;
    }

    for (int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
    }

    free(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return NULL;
    }

    // Send request type
    int request_type = GETTABLE;
    if (write_all(rtable->sockfd, &request_type, sizeof(int)) < 0) {
        perror("rtable_get_table: write request_type failed");
        return NULL;
    }

    // Receive result
    int result;
    if (read_all(rtable->sockfd, &result, sizeof(int)) < 0) {
        perror("rtable_get_table: read result failed");
        return NULL;
    }

    if (result == 0) {
        // Receive entries
        struct entry_t **entries = entries_unmarshall(rtable->sockfd);
        if (entries == NULL) {
            perror("rtable_get_table: entries_unmarshall failed");
            return NULL;
        }

        return entries;
    } else {
        return NULL;
    }
}

void rtable_free_entries(struct entry_t **entries) {
    if (entries == NULL) {
        return;
    }

    for (int i = 0; entries[i] != NULL; i++) {
        entry_destroy(entries[i]);
    }

    free(entries);
}