#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "client_stub.h"
#include "data.h"
#include "entry.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "client_stub-private.h"
#include "message-private.h"

// O programa cliente consiste num programa interativo simples, que quando executado aceita
// um comando (uma linha) do utilizador no stdin, invoca a chamada remota através da respetiva
// função do stub (Secção 3.2), imprime a resposta recebida no ecrã e volta a aceitar um novo
// comando. Uma boa forma de ler e tratar os comandos inseridos é usando as funções fgets
// e strtok. Cada comando vai ser inserido pelo utilizador numa única linha, devendo ser aceites
// os seguintes comandos (ver 1ª coluna da Tabela 1):
// put <key> <data>
// get <key>
// del <key>
// size
// getkeys
// gettable
// quit
// Note que o valor <data> no comando put deverá conter todos os caracteres após a chave
// (<key>), até ao fim da linha, podendo por isso conter espaços. Por outro lado, no
// desenvolvimento do código (nomeadamente módulos client_stub.c e table_skel.c)
// deve-se considerar que os dados <data> associados à chave <key> podem não ser strings
// e podem ser dados arbitrários (e.g., imagens, vídeos, executáveis, etc.).
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
    strtok(NULL, ":");
    
    // Conecte-se ao servidor e verifique se a conexão foi bem-sucedida
    struct rtable_t *rtable = rtable_connect(server_address_copy);
    
    if (network_connect(rtable) == -1) {
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
            char *data_given = NULL;
            size_t data_size = 0;

            if (key) {
                char buffer[1024];

                while (1) {
                    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                        if (feof(stdin)) {
                            break; // End of file, break the loop
                        } else {
                            perror("Failed to read data");
                            free(data_given);
                            return -1;
                        }
                    }

                    size_t buffer_len = strlen(buffer);
                    size_t new_size = data_size + buffer_len;
                    char *temp = realloc(data_given, new_size + 1); // +1 for the null terminator
                    if (temp == NULL) {
                        perror("Failed to allocate memory for data");
                        free(data_given);
                        return -1;
                    }
                    data_given = temp;

                    strcpy(data_given + data_size, buffer);
                    data_size = new_size;

                    // If the line ends with a newline character, break the loop
                    if (buffer[buffer_len - 1] == '\n') {
                        break;
                    }
                }
                printf("Saiu\n");

                // Remove the trailing newline character, if present
                if (data_size > 0 && data_given[data_size - 1] == '\n') {
                    data_given[data_size - 1] = '\0';
                    data_size--;
                }

                for (int i = 0; i < data_size; i++) {
                    printf("%c", data_given[i]);
                }

                struct data_t *data = data_create(data_size, data_given);
                struct entry_t *entry = entry_create(key, data);
                
                if (data == NULL) {
                    fprintf(stderr, "Failed to create data.\n");
                    if (data != NULL) data_destroy(data);
                    if (entry != NULL) entry_destroy(entry);
                    continue;
                }

                if (entry == NULL) {
                    fprintf(stderr, "Failed to create entry.\n");
                    if (entry != NULL) entry_destroy(entry);
                    continue;
                }
                // Create a MessageT structure and initialize it
                MessageT *message = malloc(sizeof(MessageT));
                if (message == NULL) {
                    free(message);
                    return -1;
                }
                message_t__init(message);

                // Populate the fields of the MessageT
                message->opcode = MESSAGE_T__OPCODE__OP_PUT;
                message->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
                message->key = key;

                // Create and populate the EntryT
                EntryT *entry_message = malloc(sizeof(EntryT));
                if (entry_message == NULL) {
                    free(entry_message);
                    return -1;
                }
                entry_t__init(entry_message);
                entry_message->key = key;

                // Create and populate the ProtobufCBinaryData
                ProtobufCBinaryData value_message;
                value_message.data = (uint8_t *)data;
                value_message.len = data->datasize;  

                entry_message->value = value_message;
                message->entry = entry_message;

                message->entry = entry_message;

                // Serialize the MessageT
                size_t message_size = message_t__get_packed_size(message);
                uint8_t *message_data = malloc(message_size);
                message_t__pack(message, message_data);
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
            char *key = strtok(NULL, "\n");
            if (key) {
                struct data_t *data = rtable_get(rtable, key);
                if (data != NULL) {
                    printf("Data for key '%s': %p\n", key, data->data);
                    data_destroy(data);
                } else {
                    fprintf(stderr, "Get operation failed.\n");
                }
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
                    printf("Key: %s, Data: %p\n", table[i]->key, table[i]->value->data);
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