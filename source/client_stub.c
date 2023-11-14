#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "client_stub.h"
#include "client_stub-private.h"
#include "message-private.h"
#include "network_client.h"

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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

    if(network_close(rtable) < 0) {
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

    MessageT *request = malloc(sizeof(MessageT));
    if (request == NULL) {
        free(request);
        return -1;
    }
    message_t__init(request);
    request->opcode = MESSAGE_T__OPCODE__OP_PUT;
    request->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    request->key = entry->key;

    EntryT *entry_message = malloc(sizeof(EntryT));
    if (entry_message == NULL) {
        free(request);
        free(entry_message);
        return -1;
    }
    entry_t__init(entry_message);
    entry_message->key = entry->key;
    entry_message->value.data = (uint8_t *) entry->value->data;
    entry_message->value.len = (size_t) entry->value->datasize;

    request->entry = entry_message;
    
    // Send the message and receive the response
    MessageT *response = network_send_receive(rtable, request);
    
    // Check if the response is valid
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_put: response not valid\n");
        free(request);
        free(response);
        free(entry_message);
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

    MessageT *request = malloc(sizeof(MessageT));
    if (request == NULL) {
        free(request);
        return NULL;
    }
    message_t__init(request);
    request->opcode = MESSAGE_T__OPCODE__OP_GET;
    request->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    request->key = key;

    EntryT *entry_message = malloc(sizeof(EntryT));
    if (entry_message == NULL) {
        free(request);
        free(entry_message);
        return NULL;
    }
    entry_t__init(entry_message);
    entry_message->key = key;

    request->entry = entry_message;

    MessageT *response = network_send_receive(rtable, request);

    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_get: response not valid\n");
        free(request);
        free(response);
        free(entry_message);
        return NULL;
    }

    struct data_t *data = data_create((int) response->value.len, (void *) response->value.data);

    free(request);
    free(response);
    free(entry_message);

    return data;
}

int rtable_del(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL) {
        return -1;
    }

    MessageT *request = malloc(sizeof(MessageT));
    if (request == NULL) {
        free(request);
        return -1;
    }
    message_t__init(request);
    request->opcode = MESSAGE_T__OPCODE__OP_DEL;
    request->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    request->key = key;

    EntryT *entry_message = malloc(sizeof(EntryT));
    if (entry_message == NULL) {
        free(request);
        free(entry_message);
        return -1;
    }
    entry_t__init(entry_message);
    entry_message->key = key;

    request->entry = entry_message;

    MessageT *response = network_send_receive(rtable, request);

    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_del: response not valid\n");
        free(request);
        free(response);
        return -1;
    }

    free(request);
    free(response);
    free(entry_message);

    return 0;
}

int rtable_size(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return -1;
    }
    
    MessageT *request = malloc(sizeof(MessageT));
    if (request == NULL) {
        free(request);
        return -1;
    }
    message_t__init(request);
    request->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    request->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rtable, request);
    
    int size = (int) response->result;

    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_size: response not valid\n");
        free(request);
        free(response);
        return -1;
    }

    free(request);
    free(response);

    return size;
}

char **rtable_get_keys(struct rtable_t *rtable) {
    if (rtable == NULL) {
        return NULL;
    }

    MessageT *request = malloc(sizeof(MessageT));
    if (request == NULL) {
        free(request);
        return NULL;
    }
    message_t__init(request);
    request->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    request->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *response = network_send_receive(rtable, request);

    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_get_keys: response not valid\n");
        return NULL;
    }

    char **keys = (char **) malloc(sizeof(char *) * (response->n_keys + 1));
    if (keys == NULL) {
        perror("rtable_get_keys: malloc failed");
        return NULL;
    }

    for (int i = 0; i < response->n_keys; i++) {
        keys[i] = strdup(response->keys[i]);
        if (keys[i] == NULL) {
            perror("rtable_get_keys: strdup failed");
            rtable_free_keys(keys);
            free(request);
            free(response);
            return NULL;
        }
    }

    keys[response->n_keys] = NULL;

    free(request);
    free(response);

    return keys;
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

    MessageT *request = malloc(sizeof(MessageT));
    if (request == NULL) {
        free(request);
        return NULL;
    }
    message_t__init(request);
    request->opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    
    MessageT *response = network_send_receive(rtable, request);

    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        fprintf(stderr, "rtable_get_table: response not valid\n");
        return NULL;
    }

    struct entry_t **entries = (struct entry_t **) malloc(sizeof(struct entry_t *) * (response->n_entries + 1));

    if (entries == NULL) {
        perror("rtable_get_table: malloc failed");
        return NULL;
    }

    for (int i = 0; i < response->n_entries; i++) {
        entries[i] = entry_create(response->entries[i]->key, data_create((int) response->entries[i]->value.len, (void *) response->entries[i]->value.data));
        if (entries[i] == NULL) {
            perror("rtable_get_table: entry_create failed");
            rtable_free_entries(entries);
            free(request);
            free(response);
            return NULL;
        }
    }

    entries[response->n_entries] = NULL;

    free(request);
    free(response);

    return entries;
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