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