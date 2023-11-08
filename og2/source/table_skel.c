#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "sdmessage.pb-c.h"
#include "data.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists){
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
    if (msg == NULL || table == NULL) {
        return -1;
    }

    // Crie uma nova estrutura MessageT para a resposta
    MessageT response = MESSAGE_T__INIT;

    switch (msg->opcode) {
        case MESSAGE_T__OPCODE__OP_SIZE:
            response.opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
            response.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            response.result = table_size(table);
            break;

        case MESSAGE_T__OPCODE__OP_DEL:
            response.opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
            response.c_type = MESSAGE_T__C_TYPE__CT_RESULT;
            response.result = table_remove(table, msg->key);
            break;

        case MESSAGE_T__OPCODE__OP_GET:
            response.opcode = MESSAGE_T__OPCODE__OP_GET + 1;
            response.c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            struct data_t *data_obtained = table_get(table, msg->key);

            if (data_obtained != NULL) {
                response.value.len = data_obtained->datasize;
                response.value.data = data_obtained->data;
            } else {
                // Se os dados não foram encontrados, defina o comprimento para 0
                response.value.len = 0;
                response.value.data = NULL;
            }
            break;

        case MESSAGE_T__OPCODE__OP_PUT:
            response.opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
            response.c_type = MESSAGE_T__C_TYPE__CT_RESULT;

            if (msg->value.len > 0 && msg->value.data != NULL) {
                struct data_t *data = data_create(msg->value.len, msg->value.data);
                response.result = table_put(table, msg->key, data);
            } else {
                // Se não houver dados válidos, retorne um resultado negativo
                response.result = -1;
            }
            break;

        case MESSAGE_T__OPCODE__OP_GETKEYS:
            response.opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
            response.c_type = MESSAGE_T__C_TYPE__CT_KEYS;
            response.keys = table_get_keys(table);
            break;

        default:
            response.opcode = MESSAGE_T__OPCODE__OP_ERROR;
            response.c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
    }

    // Copie a resposta de volta para a estrutura de mensagem original
    msg->opcode = response.opcode;
    msg->c_type = response.c_type;
    msg->result = response.result;
    msg->value = response.value;
    msg->keys = response.keys;

    return 0;
}
