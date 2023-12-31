#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "client_stub.h"
#include "data.h"
#include "entry.h"
#include "stats.h"
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
        free(rtable->server_address);
        free(rtable);
        exit(EXIT_FAILURE);
    }

    // Loop principal para aceitar comandos do usuário
    while (1) {
        char command[256];
        printf("Insira um comando (put, get, del, size, getkeys, gettable, stats ou quit): ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0';

        char *token = strtok(command, " ");
        if (token == NULL) {
            continue; 
        }
        
        if (strcmp(token, "stats") == 0){
            struct statistics_t *stats = rtable_get_stats(rtable);
            if (stats != NULL) {
                printf("total_operations: %ld\n", stats->total_operations);
                printf("total_time: %ld\n", stats->total_time);
                printf("connected_clients: %d\n", stats->connected_clients);
            } else {
                printf("Error in rtable_get_stats\n");
            }
        } else if (strcmp(token, "quit") == 0) {
            printf("Client is disconnecting and exiting.\n");
            rtable_disconnect(rtable);
            return 0;
        } else if (strcmp(token, "put") == 0) {

            char *key = strtok(NULL, " ");
            char *data_read = strtok(NULL, "\n");

            if (key && data_read) {

                int data_size = strlen(data_read) + 1;
                void *data_given = malloc(data_size);
                
                if(data_given == NULL) {
                    fprintf(stderr, "Error: Failed to allocate memory for data.\n");
                    return -1;
                }
                memcpy(data_given, data_read, data_size);
                struct data_t *data = data_create(data_size, data_given);
                struct entry_t *entry = entry_create(key, data);

                if (data == NULL || entry == NULL) {
                    fprintf(stderr, "Failed to create data or entry.\n");
                    if (data != NULL) data_destroy(data);
                    if (entry != NULL) entry_destroy(entry);
                    continue;
                }

                if (rtable_put(rtable, entry) == 0) {
                        printf("Put operation succeeded.\n");
                    } else {
                        fprintf(stderr, "Put operation failed.\n");
                    }
                free(data_given);
            } else {
                fprintf(stderr, "Invalid put command. Usage: put <key> <data>\n");
            }
        } else if (strcmp(token, "get") == 0) {
            char *key = strtok(NULL, " \n");
            if (key != NULL) {
                struct data_t *data = rtable_get(rtable, key);
                if (data != NULL) {

                    printf("Data for key '%s': '%s'\n", key, (char *) data->data);

                    data_destroy(data);
                } else {
                    printf("Error in rtable_get or key not found!\n");
                }
            }
        } else if (strcmp(token, "del") == 0) {
            char *key = strtok(NULL, "\n");
            if (key) {
                if (rtable_del(rtable, key) == 0) {
                    printf("Delete operation succeeded.\n");
                } else {
                    printf("Error in rtable_del or key not found!\n");
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
                    printf("Key: %s, data: '%s' \n", table[i]->key, (char *) table[i]->value->data);
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