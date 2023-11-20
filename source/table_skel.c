#include "client_stub.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "table.h"
#include "table-private.h"
#include "network_client.h"
#include "network_server.h"
#include "message-private.h"
#include "entry.h"
#include "data.h"
#include "stats.h"
#include "table_skel-private.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>


extern struct statistics_t *stats;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists){
    if (n_lists <= 0) {
        return NULL;
    }

    struct table_t *table = table_create(n_lists);
    if (table == NULL) {
        return NULL;
    }

    return table;
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos 
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table){
    if (table == NULL) {
        return -1;
    }

    table_destroy(table);

    return 0;
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg 
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int invoke(MessageT *msg, struct table_t *table) {

    if (msg == NULL || table == NULL || stats == NULL) {
        return -1;
    }

    int result = -1;

    switch (msg->opcode) {
        case MESSAGE_T__OPCODE__OP_STATS:
            msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;
            msg->stats = malloc(sizeof(StatsT));
            if (msg->stats == NULL) {
                return -1;
            }
            stats_t__init(msg->stats);
            msg->stats->total_operations = (int64_t) stats->total_operations;
            msg->stats->total_time = (int64_t) stats->total_time;
            msg->stats->connected_clients = (int32_t) stats->connected_clients;

            result = 0;
            break;
        case MESSAGE_T__OPCODE__OP_PUT:
            void *n_data = malloc(msg->entry->value.len);
            memcpy(n_data, msg->entry->value.data, msg->entry->value.len);
            struct data_t *new_data = data_create((int) msg->entry->value.len, n_data);
            if(new_data == NULL) {
                printf("Error: Failed to create data.\n");
                data_destroy(new_data);
                result = -1;
                break;
            }
            result = table_put(table, msg->entry->key, new_data);
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        case MESSAGE_T__OPCODE__OP_GET:
            struct data_t *data = table_get(table, msg->key);
            if (data != NULL) {
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->value.data = (uint8_t *) data->data;
                msg->value.len = (size_t) data->datasize;
                result = 0;
            }
            break;
        case MESSAGE_T__OPCODE__OP_DEL:
            result = table_remove(table, msg->key);
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        case MESSAGE_T__OPCODE__OP_SIZE:
            result = table_size(table);
            msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            msg->result = (int32_t) result;
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            char **keys = table_get_keys(table);
            int n_keys = table_size(table);
            if (keys != NULL) {
                result = 0;
            }
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            msg->keys = keys;
            msg->n_keys = (size_t) n_keys;
            break;
        case MESSAGE_T__OPCODE__OP_GETTABLE:
            msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
            msg->n_entries = (size_t) table->num_entries;
            msg->entries = malloc(msg->n_entries * sizeof(EntryT*));
            if (msg->entries == NULL) {
                return -1;
            }
            int count = 0;
            result = 0;
            for (int i = 0; i < table->size; i++) {
                for (int j = 0; j < table->lists[i]->size; j++) {
                    
                    struct entry_t *entry = table_get_n_entry(table->lists[i], j);
                    if (entry != NULL) {
                        EntryT *entry_message = malloc(sizeof(EntryT));
                        if (entry_message == NULL) {
                            free(entry_message);
                            result = -1;
                            break;
                        }
                        entry_t__init(entry_message);
                        entry_message->key = entry->key;
                        entry_message->value.data = (uint8_t *) entry->value->data;
                        entry_message->value.len = (size_t) entry->value->datasize;
                        if (count < msg->n_entries) {
                            msg->entries[count] = entry_message;
                            count++;
                        } else {
                            free(entry_message);
                            result = -1;
                            break;
                        }
                    } else {
                        result = -1;
                        break;
                    }
                }
            }
            break;
        default:
            result = -2;
            break;
    }

    if (result == -1) {
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return -1;
    } else if (result == 0) {
        msg->opcode = msg->opcode+1;
    }

    return result;
}