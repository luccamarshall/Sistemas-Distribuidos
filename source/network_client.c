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


/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) com base na
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable){
    if (rtable == NULL) {
        return -1;
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("network_connect: socket failed");
        return -1;
    }

    // Get server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(rtable->server_port);
    if (inet_pton(AF_INET, rtable->server_address, &server_addr.sin_addr) < 0) {
        perror("network_connect: inet_pton failed");
        close(sockfd);
        return -1;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("network_connect: connect failed");
        close(sockfd);
        return -1;
    }

    rtable->sockfd = sockfd;

    return 0;
}
/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Tratar de forma apropriada erros de comunicação;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg){
    if (rtable == NULL || msg == NULL) {
        return NULL;
    }

    // Send message
    if (message_send(rtable->sockfd, msg) < 0) {
        perror("network_send_receive: message_send failed");
        return NULL;
    }

    // Receive message
    MessageT *msg_resposta = message_receive(rtable->sockfd);
    if (msg_resposta == NULL) {
        perror("network_send_receive: message_receive failed");
        return NULL;
    }

    return msg_resposta;
}
/* Fecha a ligação estabelecida por network_connect().
 * Retorna 0 (OK) ou -1 (erro).
 */
int network_close(struct rtable_t *rtable){
    if (rtable == NULL) {
        return -1;
    }

    if (close(rtable->sockfd) < 0) {
        perror("network_close: close socket failed");
        return -1;
    }

    return 0;
}