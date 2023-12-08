#include "client_stub.h"
#include "sdmessage.pb-c.h"
#include "table.h"
#include "table-private.h"
#include "network_server.h"
#include "message-private.h"
#include "entry.h"
#include "data.h"
#include "table_skel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <zookeeper/zookeeper.h>

int main(int argc, char *argv[]) {
        if (argc != 4) {
        fprintf(stderr, "Uso: %s <IP>:<porto> <porto> <num_lists>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *zk_address = argv[1];
    short port = atoi(argv[2]);
    int num_lists = atoi(argv[3]);

    // Inicialize a tabela
    struct table_t *table = table_skel_init(num_lists); 

    if (table == NULL) {
        fprintf(stderr, "Erro ao criar a tabela.\n");
        exit(EXIT_FAILURE);
    }

    printf("Server ready, waiting for connections\n");

    // Inicialize o socket do servidor
    int server_socket = network_server_init(port);

    if (server_socket == -1) {
        fprintf(stderr, "Erro ao inicializar o socket do servidor.\n");
        table_destroy(table);
        exit(EXIT_FAILURE);
    }

    // Conecte-se ao ZooKeeper com o IP e porto fornecidos
    zhandle_t *zh = connect_zookeeper(zk_address);

    // Create a ZNode under /chain
    char path_buffer[20];
    int buffer_len = sizeof(path_buffer);
    int rc = zoo_acreate(zh, "/chain/node", "", 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, path_buffer, buffer_len);

    // Check if ZNode creation was successful
    if (rc != ZOK) {
        fprintf(stderr, "Error creating ZNode: %s\n", zerror(rc));
        network_server_close(server_socket);
        table_destroy(table);
        exit(EXIT_FAILURE);
    }

    // Store the ZNode id
    char *znode_id = strdup(path_buffer);

    // Get and watch the children of /chain
    struct String_vector children;
    rc = zoo_awget_children(zh, "/chain", watch_children, NULL, &children);

    // Check if getting children was successful
    if (rc != ZOK) {
        fprintf(stderr, "Error getting children of /chain: %s\n", zerror(rc));
        exit(EXIT_FAILURE);
    }

    // Handle successor and predecessor nodes
    handle_successor_predecessor(&children, znode_id);


    // Execute o loop principal do servidor
    int result = network_main_loop(server_socket, table);

    if (result == -1) {
        fprintf(stderr, "Erro durante a execução do loop principal do servidor.\n");
    }

    // Feche o socket do servidor e libere recursos
    network_server_close(server_socket);
    table_destroy(table);

    return 0;
}

void handle_successor_predecessor(struct String_vector *children, char *znode_id) {
    // Sort the children nodes
    sort_children(children);

    // Find the current node's position
    int position = find_position(children, znode_id);

    // If there's a successor node (i.e., a node with a higher id)
    if (position < children->count - 1) {
        // Connect to the successor node and store its id and socket
        struct rtable_t *successor_rtable = rtable_connect(children->data[position + 1]);
        if (successor_rtable == NULL) {
            fprintf(stderr, "Error connecting to successor node.\n");
            exit(EXIT_FAILURE);
        }
        // Store the successor's socket and id
        int successor_sockfd = successor_rtable->sockfd;
        char *successor_address = successor_rtable->server_address;
        int successor_port = successor_rtable->server_port;

    }

    // // If there's a predecessor node (i.e., a node with a lower id)
    // if (position > 0) {
    //     // Connect to the predecessor node, get a copy of its table, and update the local table
    //     update_table_from_predecessor(children->data[position - 1]);
    // }
}

void sort_children(struct String_vector *children) {
    // Simple bubble sort
    for (int i = 0; i < children->count - 1; i++) {
        for (int j = 0; j < children->count - i - 1; j++) {
            if (strcmp(children->data[j], children->data[j + 1]) > 0) {
                char *temp = children->data[j];
                children->data[j] = children->data[j + 1];
                children->data[j + 1] = temp;
            }
        }
    }
}

int find_position(struct String_vector *children, char *znode_id) {
    for (int i = 0; i < children->count; i++) {
        if (strcmp(children->data[i], znode_id) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

//connect to the successor node
void connect_successor(char *successor_id) {
    struct rtable_t *successor_rtable = rtable_connect(successor_id);
    if (successor_rtable == NULL) {
        fprintf(stderr, "Error connecting to successor node.\n");
        exit(EXIT_FAILURE);
    }
}

// void update_table_from_predecessor(char *predecessor_id) {
//     // Parse the predecessor_id to get the address and port
//     char *address = strtok(predecessor_id, ":");
//     int port = atoi(strtok(NULL, ":"));

//     // Connect to the predecessor server
//     struct rtable_t *predecessor_rtable = rtable_connect(port);
//     if (predecessor_rtable == NULL) {
//         fprintf(stderr, "Error connecting to predecessor server.\n");
//         // Handle error
//     }

//     // Get a copy of the predecessor's table
//     struct table_t *predecessor_table = ;
//     if (predecessor_table == NULL) {
//         fprintf(stderr, "Error getting table from predecessor server.\n");
//         // Handle error
//     }

//     // Update the local table
//     table_update(table, predecessor_table);

//     // Disconnect from the predecessor server and free the predecessor_rtable
//     rtable_disconnect(predecessor_rtable);
//     free(predecessor_rtable);
// }