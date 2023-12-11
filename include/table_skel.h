#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "table.h"
#include <zookeeper/zookeeper.h>
#include "sdmessage.pb-c.h"

void sort_children(struct String_vector *children);

int find_position(struct String_vector *children, char *znode_id);

void connect_predecessor(char *predecessor_id);

void connect_successor(char *successor_id);

void zookeeper_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcher_ctx);

zhandle_t *connect_zookeeper(const char *zk_address);

void my_completion(int rc, const struct String_vector *strings, const void *data);

void watch_children(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx);

void handle_successor_predecessor(struct String_vector *children, char *znode_id);

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists, const char *zk_address);

/* Liberta toda a memória ocupada pela tabela e todos os recursos 
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table);

/* Executa na tabela table a operação indicada pelo opcode contido em msg 
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int invoke(MessageT *msg, struct table_t *table);

#endif
