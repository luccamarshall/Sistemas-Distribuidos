#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <protobuf-c/protobuf-c.h>
#include "network_client.h"
#include "network-private.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "data.h"
#include "entry.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

struct rtable_t *rtable_connect(char *address_port) {
    if (address_port == NULL) {
        perror("Invalid address_port");
        return NULL;
    }

    // Create a new rtable_t
    struct rtable_t *rtable = (struct rtable_t *)malloc(sizeof(struct rtable_t));
    if (rtable == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    // Parse the address_port string
    char *address = strtok(address_port, ":");
    char *port = strtok(NULL, ":");

    if (address == NULL || port == NULL) {
        perror("Invalid address_port");
        free(rtable);
        return NULL;
    }

    // Copy the address to the rtable_t
    rtable->server_address = (char *)malloc(strlen(address) + 1);
    if (rtable->server_address == NULL) {
        perror("Memory allocation error");
        free(rtable);
        return NULL;
    }
    strcpy(rtable->server_address, address);

    // Connect to the server
    if (network_connect(rtable, atoi(port)) == -1) {
        perror("Failed to connect to the server");
        free(rtable->server_address);
        free(rtable);
        return NULL;
    }

    return rtable;
}

int rtable_disconnect(struct rtable_t *rtable){
    if(rtable == NULL){
        perror("Erro ao terminar a associação entre o cliente e o servidor");
        return -1;
    }

    if(network_close(rtable) == -1){
        perror("Erro ao terminar a associação entre o cliente e o servidor");
        return -1;
    }

    free(rtable);
    return 0;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    if (rtable == NULL || entry == NULL) {
        perror("Error: Invalid rtable or entry");
        return -1;
    }
    // Create a new MessageT and initialize it
    MessageT message = MESSAGE_T__INIT;

    // Populate the fields of the MessageT
    message.opcode = MESSAGE_T__OPCODE__OP_PUT;
    message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    message.key = entry->key;

    // Create and populate the EntryT
    EntryT entry_message = ENTRY_T__INIT;
    entry_message.key = entry->key;
    
    // Create and populate the ProtobufCBinaryData
    ProtobufCBinaryData value_message;
    value_message.data = entry->value->data;
    value_message.len = entry->value->datasize;

    entry_message.value = value_message;
    message.entry = &entry_message;
    
    // Serialize the MessageT
    uint8_t *message_data = serialize_message(&message);    

    if (message_data == NULL) {
        perror("Error: Failed to serialize message");
        return -1;
    }
    MessageT *response = network_send_receive(rtable, &message);

    free(message_data);

    if (response == NULL) {
        perror("Error: Failed to send the message");
        return -1;
    }

    return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL) {
        perror("Invalid parameters");
        return NULL;
    }

    // Create a MessageT to hold the request
    MessageT request = MESSAGE_T__INIT;
    request.opcode = MESSAGE_T__OPCODE__OP_GET;
    request.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    request.key = key;

    // Send the request and receive the response
    MessageT *response = network_send_receive(rtable, &request);

    if (response == NULL) {
        perror("Failed to retrieve the element");
        return NULL;
    }

    // Check if the response contains a valid data
    if (response->opcode != MESSAGE_T__OPCODE__OP_ERROR && response->c_type == MESSAGE_T__C_TYPE__CT_VALUE) {
        void *data = malloc(response->value.len);
        memcpy(data, response->value.data, response->value.len);
        struct data_t *data_t = data_create(response->value.len, data);
        if (data == NULL) {
            perror("Memory allocation error");
            return NULL;
        }

        printf("Data: ");
        char *str = (char*)data;
        for (int i = 0; str[i] != '\0'; i++) {
            printf("%c", str[i]);
        }
        printf("\n");
        return data_t;

        // data->datasize = response->value.len;
        // data->data = malloc(data->datasize);
        // if (data->data == NULL) {
        //     free(data);
        //     perror("Memory allocation error");
        //     return NULL;
        // }

        // memcpy(data->data, response->value.data, data->datasize);
        
        // return data;
    }

    // Handle the case when the response is not valid
    perror("Failed to retrieve the element");
    return NULL;
}

int rtable_del(struct rtable_t *rtable, char *key) {
    if (rtable == NULL || key == NULL) {
        perror("Invalid parameters");
        return -1;
    }

    // Create a MessageT to hold the request
    MessageT request = MESSAGE_T__INIT;
    request.opcode = MESSAGE_T__OPCODE__OP_DEL;
    request.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    request.key = key;

    // Send the request and receive the response
    MessageT *response = network_send_receive(rtable, &request);

    if (response == NULL) {
        perror("Failed to delete the element");
        return -1;
    }

    // Check if the response contains a valid result
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR || response->c_type != MESSAGE_T__C_TYPE__CT_RESULT) {
        perror("Failed to delete the element");
        return -1;
    }

    int result = response->result;
    free(response);

    if (result == 0) {
        return 0; // Element deleted successfully
    } else {
        perror("Failed to delete the element");
        return -1; // Element not found or an error occurred
    }
}

int rtable_size(struct rtable_t *rtable) {

    if (rtable == NULL) {
        perror("Invalid rtable");
        return -1;
    }

    // Create a MessageT to hold the request
    MessageT request = MESSAGE_T__INIT;
    request.opcode = MESSAGE_T__OPCODE__OP_SIZE;

    // Send the request and receive the response
    MessageT *response = network_send_receive(rtable, &request);

    if (response == NULL) {
        perror("Failed to retrieve the size");
        return -1;
    }

    // Check if the response contains a valid result
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR || response->c_type != MESSAGE_T__C_TYPE__CT_RESULT) {
        perror("Failed to retrieve the size");
        return -1;
    }

    int result = response->result;
    free(response);

    return result;
}

char **rtable_get_keys(struct rtable_t *rtable) {

    if (rtable == NULL) {
        perror("Invalid parameter");
        return NULL;
    }

    // Create a MessageT to hold the request
    MessageT request = MESSAGE_T__INIT;
    request.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;

    // Send the request and receive the response
    MessageT *response = network_send_receive(rtable, &request);

    if (response == NULL) {
        perror("Failed to retrieve keys");
        return NULL;
    }

    // Check if the response contains a valid list of keys
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR || response->c_type != MESSAGE_T__C_TYPE__CT_KEYS) {
        perror("Failed to retrieve keys");
        return NULL;
    }

    char **keys = response->keys;

    free(response);

    return keys;
}

void rtable_free_keys(char **keys){
    if(keys == NULL){
        perror("Erro ao libertar a memória alocada para as keys");
        return;
    }

    for(int i = 0; keys[i] != NULL; i++){
        free(keys[i]);
    }

    free(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    if (rtable == NULL) {
        perror("Invalid parameter");
        return NULL;
    }

    // Create a MessageT to hold the request
    MessageT request = MESSAGE_T__INIT;
    request.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;

    // Send the request and receive the response
    MessageT *response = network_send_receive(rtable, &request);

    if (response == NULL) {
        perror("Failed to retrieve the table");
        return NULL;
    }

    // Check if the response contains a valid table
    if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR || response->c_type != MESSAGE_T__C_TYPE__CT_TABLE) {
        perror("Failed to retrieve the table");
        return NULL;
    }

    EntryT **table = response->entries;
    struct entry_t **entries = (struct entry_t **)malloc(sizeof(struct entry_t *) * (response->n_entries + 1));
    
    if (entries == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    for (int i = 0; table[i] != NULL; i++) {
        EntryT *entry = table[i];
        struct data_t *data = (struct data_t *)malloc(sizeof(struct data_t));
        if (data == NULL) {
            perror("Memory allocation error");
            return NULL;
        }
        
        data->datasize = entry->value.len;
        data->data = (char *)malloc(data->datasize);
        if (data->data == NULL) {
            free(data);
            perror("Memory allocation error");
            return NULL;
        }

        memcpy(data->data, entry->value.data, data->datasize);

        entries[i] = entry_create(entry->key, data);
    }
    free(response);

    return entries;
}

void rtable_free_entries(struct entry_t **entries){
    if(entries == NULL){
        perror("Erro ao libertar a memória alocada para as entries");
        return;
    }

    for(int i = 0; entries[i] != NULL; i++){
        free(entries[i]->key);
        data_destroy(entries[i]->value);
        free(entries[i]);
    }

    free(entries);
}