#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "network_client.h"
#include "client_stub-private.h"
#include "message-private.h"



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
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if (rtable == NULL || msg == NULL) {
        return NULL;
    }
    
    int sockfd = rtable->sockfd;

    // Serialize message
    size_t msg_size = message_t__get_packed_size(msg);
    void *msg_buf = malloc(msg_size);
    if (msg_buf == NULL) {
        perror("network_send_receive: malloc failed");
        return NULL;
    }
    message_t__pack(msg, (uint8_t *) msg_buf);
    
    // Send message
    uint16_t msg_size_n = htons((uint16_t) msg_size);
    if (write_all(sockfd, &msg_size_n, sizeof(uint16_t)) == -1) {
        // Tratar erro de envio
        free(msg_buf);
        return NULL;
    }
    
    if (write_all(sockfd, msg_buf, msg_size) == -1) {
        // Tratar erro de envio
        free(msg_buf);
        return NULL;
    }
    
    // Receive message size
    uint16_t response_size;
    if (read_all(sockfd, &response_size, sizeof(uint16_t)) == -1) {
       printf("Error reading response size\n");
        return NULL;
    }
    response_size = ntohs(response_size);
    
    // Aloca memória para o buffer de resposta
    uint8_t *response_buffer = malloc(response_size);
    if (response_buffer == NULL) {
        // Tratar erro de alocação de memória
        return NULL;
    }

    // Recebe o buffer de resposta
    if (read_all(sockfd, response_buffer, response_size) == -1) {
        // Tratar erro de recebimento
        free(response_buffer);
        return NULL;
    }
    // Deserialize response
    MessageT *response = message_t__unpack(NULL, (size_t) response_size, response_buffer);
    if (response == NULL) {
        perror("network_send_receive: message_t__unpack failed");
        free(msg_buf);
        free(response_buffer);
        return NULL;
    }

    free(msg_buf);
    free(response_buffer);

    return response;
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