#include "client_stub.h"
#include "sdmessage.pb-c.h"
#include "table.h"
#include "table-private.h"
#include "network_client.h"
#include "network_client-private.h"
#include "message-private.h"
#include "entry.h"
#include "data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

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
int invoke(MessageT *msg, struct table_t *table){
    if (msg == NULL || table == NULL) {
        return -1;
    }

    int result = -1;
    struct data_t *data = NULL;
    struct entry_t *entry = NULL;

    switch (msg->opcode) {
        case OC_PUT:
            result = table_put(table, msg->content.entry->key, msg->content.entry->value);
            break;
        case OC_COND_PUT:
            result = table_conditional_put(table, msg->content.entry->key, msg->content.entry->value);
            break;
        case OC_GET:
            data = table_get(table, msg->content.key);
            if (data != NULL) {
                result = 0;
            }
            break;
        case OC_DEL:
            result = table_del(table, msg->content.key);
            break;
        case OC_SIZE:
            result = table_size(table);
            break;
        case OC_GET_KEYS:
            result = table_get_keys(table, &entry);
            break;
        default:
            break;
    }

    if (result == -1) {
        return -1;
    }

    msg->opcode = result;
    msg->c_type = CT_RESULT;

    switch (msg->opcode) {
        case OC_PUT:
            msg->content.result = 0;
            break;
        case OC_COND_PUT:
            msg->content.result = 0;
            break;
        case OC_GET:
            msg->content.data = data;
            break;
        case OC_DEL:
            msg->content.result = 0;
            break;
        case OC_SIZE:
            msg->content.result = result;
            break;
        case OC_GET_KEYS:
            msg->content.keys = entry;
            break;
        default:
            break;
    }

    return 0;
