#include "client_stub.h"
#include "client_stub-private.h"
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

#include <zookeeper/zookeeper.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

extern struct statistics_t *stats;

zhandle_t *zh;

void sort_children(struct String_vector *children)
{
    int i, j;
    for (i = 0; i < children->count - 1; i++)
    {
        for (j = 0; j < children->count - i - 1; j++)
        {
            if (strcmp(children->data[j], children->data[j + 1]) > 0)
            {
                char *temp = children->data[j];
                children->data[j] = children->data[j + 1];
                children->data[j + 1] = temp;
            }
        }
    }
}

int find_position(struct String_vector *children, char *znode_id)
{
    int i;
    for (i = 0; i < children->count; i++)
    {
        if (strcmp(children->data[i], znode_id) == 0)
        {
            return i;
        }
    }
    return -1;
}

void connect_predecessor(char *predecessor_id) {
    // Connect to the predecessor server
    struct rtable_t *predecessor_rtable = rtable_connect(predecessor_id);
    if (predecessor_rtable == NULL)
    {
        fprintf(stderr, "Error connecting to predecessor server.\n");
        exit(EXIT_FAILURE);
    }

    predecessor_rtable = predecessor_rtable;
}

void connect_successor(char *successor_id) {
    // Connect to the successor server
    struct rtable_t *successor_rtable = rtable_connect(successor_id);
    if (successor_rtable == NULL)
    {
        fprintf(stderr, "Error connecting to successor server.\n");
        exit(EXIT_FAILURE);
    }

    successor_rtable = successor_rtable;
}

void zookeeper_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcher_ctx)
{
    // Handle ZooKeeper events, if necessary
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            printf("Connected to ZooKeeper.\n");
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            printf("Session expired.\n");
        }
    }
    else if (type == ZOO_CREATED_EVENT)
    {
        printf("Node created: %s\n", path);
    }
    else if (type == ZOO_DELETED_EVENT)
    {
        printf("Node removed: %s\n", path);
    }
    else if (type == ZOO_CHANGED_EVENT)
    {
        printf("Node changed: %s\n", path);
    }
    else if (type == ZOO_CHILD_EVENT)
    {
        printf("Child node changed: %s\n", path);
    }
    else if (type == ZOO_NOTWATCHING_EVENT)
    {
        printf("No longer watching: %s\n", path);
    }
    else
    {
        printf("Unknown event: %d\n", type);
    }
}

zhandle_t *connect_zookeeper(const char *zk_address)
{
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zh = zookeeper_init(zk_address, zookeeper_watcher, 10000, 0, NULL, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error initializing ZooKeeper client.\n");
        exit(EXIT_FAILURE);
    }
    return zh;
}

void my_completion(int rc, const struct String_vector *strings, const void *data) {
    if (rc != ZOK) {
        fprintf(stderr, "Error %d for zoo_awget_children.\n", rc);
        return;
    }

    printf("Children of /chain node:\n");
    for (int i = 0; i < strings->count; i++) {
        printf("%s\n", strings->data[i]);
    }
}

void watch_children(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx)
{
    // Handle ZooKeeper events, if necessary
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            printf("Connected to ZooKeeper.\n");
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            printf("Session expired.\n");
        }
    }
    else if (type == ZOO_CREATED_EVENT)
    {
        printf("Node created: %s\n", path);
    }
    else if (type == ZOO_DELETED_EVENT)
    {
        printf("Node removed: %s\n", path);
    }
    else if (type == ZOO_CHANGED_EVENT)
    {
        printf("Node changed: %s\n", path);
    }
    else if (type == ZOO_CHILD_EVENT)
    {
        printf("Child node changed: %s\n", path);
    }
    else if (type == ZOO_NOTWATCHING_EVENT)
    {
        printf("No longer watching: %s\n", path);
    }
    else
    {
        printf("Unknown event: %d\n", type);
    }

    // Get and watch the children of /chain
    struct String_vector children;
    int rc = zoo_awget_children(zh, "/chain", watch_children, NULL, my_completion, NULL);
    
    const char *zerror(int rc);

    // Check if getting children was successful
    if (rc != ZOK)
    {
        fprintf(stderr, "Error getting children of /chain: %s\n", zerror(rc));
        exit(EXIT_FAILURE);
    }

    // Handle successor and predecessor nodes
    handle_successor_predecessor(&children, (char *)watcherCtx);
}

void handle_successor_predecessor(struct String_vector *children, char *znode_id)
{
    // Sort the children
    sort_children(children);

    // Find the position of this node
    int position = find_position(children, znode_id);

    // Get the id of the predecessor node
    char *predecessor_id = NULL;
    if (position == 0)
    {
        predecessor_id = strdup(children->data[children->count - 1]);
    }
    else
    {
        predecessor_id = strdup(children->data[position - 1]);
    }

    // Get the id of the successor node
    char *successor_id = NULL;
    if (position == children->count - 1)
    {
        successor_id = strdup(children->data[0]);
    }
    else
    {
        successor_id = strdup(children->data[position + 1]);
    }

    // Connect to the predecessor node
    connect_predecessor(predecessor_id);

    // Connect to the successor node
    connect_successor(successor_id);
}



struct table_t *table_skel_init(int n_lists, const char *zk_address)
{
    if (n_lists <= 0 || zk_address == NULL)
    {
        return NULL;
    }

    struct table_t *table = table_create(n_lists);
    if (table == NULL)
    {
        return NULL;
    }

    // Initialize the ZooKeeper client
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zh = zookeeper_init(zk_address, zookeeper_watcher, 10000, 0, NULL, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error initializing ZooKeeper client.\n");
        table_destroy(table);
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
            // sleep(5);
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
            data_destroy(new_data);
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            break;
        case MESSAGE_T__OPCODE__OP_GET:
            struct data_t *data = table_get(table, msg->key);
            if (data != NULL) {
                msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
                msg->value.data = malloc(data->datasize);
                memcpy(msg->value.data, data->data, data->datasize);
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
            // sleep(5);
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
                            entry_destroy(entry);
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