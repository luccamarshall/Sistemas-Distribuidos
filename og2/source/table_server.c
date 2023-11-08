#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "table.h"
#include "table_skel.h"
#include "network_server.h"
#include "network-private.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <porto> <num_lists>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    short port = atoi(argv[1]);
    int num_lists = atoi(argv[2]);

    // Inicialize a tabela
    struct table_t *table = table_skel_init(num_lists); 

    if (table == NULL) {
        fprintf(stderr, "Erro ao criar a tabela.\n");
        exit(EXIT_FAILURE);
    }

    printf("Servidor inicializado... esperando conecções\n");

    // Inicialize o socket do servidor
    int server_socket = network_server_init(port);

    if (server_socket == -1) {
        fprintf(stderr, "Erro ao inicializar o socket do servidor.\n");
        table_destroy(table);
        exit(EXIT_FAILURE);
    }

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