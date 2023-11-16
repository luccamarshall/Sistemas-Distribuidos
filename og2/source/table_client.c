/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "client_stub.h"
#include "data.h"
#include "entry.h"
#include "network-private.h"
#include "network_client.h"
#include "message-private.h"
#include "client_stub-private.h"

#include "table_skel.h"

int main(int argc, char *argv[]) {
    // Verifique o número correto de argumentos da linha de comando (endereço e porta do servidor)
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <endereço_do_servidor>:<porta_do_servidor>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Conecte-se ao servidor usando o endereço e a porta fornecidos
    char *server_address = argv[1];
    char *server_address_copy = strdup(server_address);
    server_address = strtok(server_address, ":");
    int server_port = atoi(strtok(NULL, ":"));
    
    // Conecte-se ao servidor e verifique se a conexão foi bem-sucedida
    struct rtable_t *rtable = rtable_connect(server_address_copy);
    
    if (network_connect(rtable, server_port) == -1) {
        fprintf(stderr, "Erro ao criar a ligação ao servidor\n");
        free(rtable);
        exit(EXIT_FAILURE);
    }

    // Loop principal para aceitar comandos do usuário
    while (1) {
        char command[256];
        printf("Insira um comando (put, get, del, size, getkeys, gettable ou quit): ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0'; 

        char *token = strtok(command, " ");
        if (token == NULL) {
            continue; 
        }
        
        if (strcmp(token, "quit") == 0) {
            printf("Client is disconnecting and exiting.\n");
            rtable_disconnect(rtable);
            return 0;
        } else if (strcmp(token, "put") == 0) {

            char *key = strtok(NULL, " \n");
            char *data_given = strtok(NULL, "\n");
            if (key && data_given) {
                struct data_t *data = data_create(strlen(data_given), data_given);
                struct entry_t *entry = entry_create(key, data);
                
                if (data == NULL || entry == NULL) {
                    fprintf(stderr, "Failed to create data or entry.\n");
                    if (data != NULL) data_destroy(data);
                    if (entry != NULL) entry_destroy(entry);
                    continue;
                }
                // Create a MessageT structure and initialize it
                MessageT message = MESSAGE_T__INIT;

                // Populate the fields of the MessageT
                message.opcode = MESSAGE_T__OPCODE__OP_PUT;
                message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
                message.key = key;

                // Create and populate the EntryT
                EntryT entry_message = ENTRY_T__INIT;
                entry_message.key = key;

                // Create and populate the ProtobufCBinaryData
                ProtobufCBinaryData value_message;
                value_message.data = (uint8_t *)data;
                value_message.len = data->datasize;  

                entry_message.value = value_message;
                message.entry = &entry_message;

                // Serialize the MessageT
                size_t message_size = message_t__get_packed_size(&message);
                uint8_t *message_data = serialize_message(&message);
                int server_socket = rtable->sockfd;
                ssize_t bytes = write_all(server_socket, message_data, message_size);

                if (bytes < 0) { 
                    fprintf(stderr, "Error: Failed to serialize message.\n");
                } else if (bytes != message_size) {
                    fprintf(stderr, "Error: Incomplete message sent.\n");
                } else {

                    if (rtable_put(rtable, entry) == 0) {
                        printf("Put operation succeeded.\n");
                    } else {
                        fprintf(stderr, "Put operation failed.\n");
                    }

                    free(message_data);
                }
            } else {
                fprintf(stderr, "Invalid put command. Usage: put <key> <data>\n");
            }
        } else if (strcmp(token, "get") == 0) {
            char *key = strtok(NULL, " \n");
            if (key != NULL) {
                struct data_t *data = rtable_get(rtable, key);
                char *str = (char*)data;
                for (int i = 0; str[i] != '\0'; i++) {
                    printf("%c", str[i]);
                }
                printf("\n");
                } else {
                    fprintf(stderr, "Get operation failed.\n");
                }
        } else if (strcmp(token, "del") == 0) {
            char *key = strtok(NULL, "\n");
            if (key) {
                if (rtable_del(rtable, key) == 0) {
                    printf("Delete operation succeeded.\n");
                } else {
                    fprintf(stderr, "Delete operation failed.\n");
                }
            }
        } else if (strcmp(token, "size") == 0) {
            int size = rtable_size(rtable);
            if (size != -1) {
                printf("Number of elements in the table: %d\n", size);
            } else {
                fprintf(stderr, "Failed to get the table size.\n");
            }
        } else if (strcmp(token, "getkeys") == 0) {
            char **keys = rtable_get_keys(rtable);
            if (keys != NULL) {
                printf("Keys in the table:\n");
                for (int i = 0; keys[i] != NULL; i++) {
                    printf("%s\n", keys[i]);
                }
                rtable_free_keys(keys);
            } else {
                fprintf(stderr, "Failed to get keys from the table.\n");
            }
        } else if (strcmp(token, "gettable") == 0) {
            struct entry_t **table = rtable_get_table(rtable);
            if (table != NULL) {
                printf("Entries in the table:\n");
                for (int i = 0; table[i] != NULL; i++) {
                    char *str = (char*)table[i]->value->data;
                    printf("Key: %s, \n", table[i]->key);
                    printf("Data: ");
                    for (int i = 0; str[i] != '\0'; i++) {
                        printf("%c", str[i]);
                    }
                    printf("\n");
                }
                rtable_free_entries(table);
            } else {
                fprintf(stderr, "Failed to get the table entries.\n");
            }
        } else {
            printf("Invalid command. Available commands: put, get, del, size, getkeys, gettable, or quit.\n");
        }
    }

    rtable_disconnect(rtable);
    
    return 0;
}