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

#

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int network_server_init(short port){
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("network_server_init: socket failed");
        return -1;
    }

    // Bind socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("network_server_init: bind failed");
        close(sockfd);
        return -1;
    }

    // Listen
    if (listen(sockfd, 0) < 0) {
        perror("network_server_init: listen failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

/* A função network_main_loop() deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada
     na tabela table;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 * A função não deve retornar, a menos que ocorra algum erro. Nesse
 * caso retorna -1.
 */
int network_main_loop(int listening_socket, struct table_t *table){
    if (listening_socket < 0 || table == NULL) {
        return -1;
    }

    while (1) {
        // Accept connection
        int client_socket = accept(listening_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("network_main_loop: accept failed");
            return -1;
        }

        // Receive message
        MessageT *msg = network_receive(client_socket);
        if (msg == NULL) {
            perror("network_main_loop: network_receive failed");
            close(client_socket);
            return -1;
        }

        // Invoke message
        int result = invoke(msg, table);
        if (result == -1) {
            perror("network_main_loop: invoke failed");
            close(client_socket);
            return -1;
        }

        // Send message
        result = network_send(client_socket, msg);
        if (result == -1) {
            perror("network_main_loop: network_send failed");
            close(client_socket);
            return -1;
        }

        // Free message
        message_t__free_unpacked(msg, NULL);

        // Close connection
        close(client_socket);
    }

    return 0;
}

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket){
    if (client_socket < 0) {
        return NULL;
    }

    // Receive message
    MessageT *msg = message_receive(client_socket);
    if (msg == NULL) {
        perror("network_receive: message_receive failed");
        return NULL;
    }

    return msg;
}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg){
    if (client_socket < 0 || msg == NULL) {
        return -1;
    }

    // Send message
    if (message_send(client_socket, msg) < 0) {
        perror("network_send: message_send failed");
        return -1;
    }

    return 0;
}

/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket){
    if (socket < 0) {
        return -1;
    }

    if (close(socket) < 0) {
        perror("network_server_close: close socket failed");
        return -1;
    }

    return 0;
}
