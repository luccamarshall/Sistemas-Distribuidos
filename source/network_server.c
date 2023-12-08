#include "network_server.h"
#include "client_stub.h"
#include "sdmessage.pb-c.h"
#include "table.h"
#include "table-private.h"
#include "network_client.h"
#include "message-private.h"
#include "entry.h"
#include "data.h"
#include "stats.h"
#include "table_skel.h"

#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

pthread_cond_t cv;
int ret;
pthread_mutex_t write_lock;
int num_readers, writing;

// Global variable declaration
struct statistics_t *stats;

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

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv, NULL);
    num_readers = 0;
    writing = 0;
    
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

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket) {
    if (client_socket < 0) {
        return NULL;
    }
    
    // Receive message size
    uint16_t response_size;
    if (read_all(client_socket, &response_size, sizeof(uint16_t)) == -1) {
        // Tratar erro de recebimento
        return NULL;
    }
    uint16_t msg_size = ntohs(response_size);

    uint8_t *response_buffer = malloc(msg_size);
    if (response_buffer == NULL) {
        free(response_buffer);
        return NULL;
    }
    
    if (read_all(client_socket, response_buffer, msg_size) == -1) {
        // Tratar erro de recebimento
        free(response_buffer);
        printf("Error reading response message\n");
        return NULL;
    }
    
    MessageT *received_message = message_t__unpack(NULL, (size_t) msg_size, response_buffer);

    free(response_buffer);

    return received_message;
}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg) {
    if (client_socket < 0 || msg == NULL) {
        return -1;
    }

    // Serialize message
    size_t msg_size = message_t__get_packed_size(msg);
    void *msg_buf = malloc(msg_size);
    if (msg_buf == NULL) {
        perror("network_send: malloc failed");
        return -1;
    }
    size_t packed_size = message_t__pack(msg, (uint8_t *) msg_buf);
    if (packed_size != msg_size) {
        perror("network_send: message_t__pack failed");
        free(msg_buf);
        return -1;
    }

    // Send message
    uint16_t msg_size_n = htons((uint16_t) msg_size);
    
    if (write_all(client_socket, &msg_size_n, sizeof(uint16_t)) == -1) {
        // Tratar erro de envio
        free(msg_buf);
        return -1;
    }
    
    if (write_all(client_socket, msg_buf, msg_size) == -1) {
        // Tratar erro de envio
        free(msg_buf);
        return -1;
    }

    free(msg_buf);

    return 0;
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

    stats = stats_init();
    
    while (1) {
        // Accept connection
        if(stats->connected_clients == 0){
            printf("Waiting for a client to connect...\n");
        }
        int client_socket = accept(listening_socket, NULL, NULL);

        if (client_socket < 0) {
            perror("network_main_loop: accept failed");
            return -1;
        } else {
            printf("A client has connected\n");
            stats_update_clients(stats, 1);
        }

        // Create a new thread to handle the client's requests
        struct client_thread_args *args = malloc(sizeof(struct client_thread_args));
        args->client_socket = client_socket;
        args->table = table;
        args->stats = stats;
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, args);
        pthread_detach(thread);
    }

    return 0;
}

void *handle_client(void *arg) {
    struct client_thread_args *args = (struct client_thread_args *)arg;
    stats = args->stats; 
    int client_socket = args->client_socket;
    struct table_t *table = args->table;

    MessageT *msg = NULL;
    int result = 0;

    struct timeval start, end;

    while(result == 0) {
        while(msg == NULL) {
            msg = network_receive(client_socket);
        }

        // Start the timer
        long time = 0;
        if (msg->opcode != MESSAGE_T__OPCODE__OP_STATS) {
            gettimeofday(&start, NULL);
        }


        // pthread_mutex_lock(&lock);

        // if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT ||
        //     msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        //     while(num_readers > 0 || writing) {
        //         pthread_cond_wait(&cv, &lock);
        //     }
        //     writing = 1;
        // } else {
        //     while(writing) {
        //         pthread_cond_wait(&cv, &lock);
        //     }
        //     num_readers++;
        // }

        // pthread_mutex_unlock(&lock);
        
        // result = invoke(msg, table);

        // pthread_mutex_lock(&lock);

        // if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT+1 ||
        //     msg->opcode == MESSAGE_T__OPCODE__OP_DEL+1) {
        //     writing = 0;
        //     pthread_cond_broadcast(&cv);
        // } else {
        //     num_readers--;
        //     if (num_readers == 0) {
        //         pthread_cond_signal(&cv);
        //     }
        // }

        // pthread_mutex_unlock(&lock);

        if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT ||
            msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
            // Lock the mutex before performing the write operations
            pthread_mutex_lock(&write_lock);

            // Perform the local write operation
            result = invoke(msg, table);

            // Perform the remote write operation
            if (successor_rtable != NULL) {
                if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT) {
                    struct entry_t *entry = // Create entry from msg
                    int result = rtable_put(successor_rtable, entry);
                    if (result == -1) {
                        fprintf(stderr, "Error replicating put operation on successor server.\n");
                    }
                } else if (msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
                    char *key = // Get key from msg
                    int result = rtable_del(successor_rtable, key);
                    if (result == -1) {
                        fprintf(stderr, "Error replicating del operation on successor server.\n");
                    }
                }
            }

            // Unlock the mutex after the write operations
            pthread_mutex_unlock(&write_lock);
        } else {
            // Perform the read operation
            result = invoke(msg, table);
        }
            

        if (result == -2) {
            printf("A client has disconnected\n");
            stats_update_clients(stats, -1);
            close(client_socket);
            free(args);
            return NULL;
        }

        // End the timer
        if (msg->opcode != MESSAGE_T__OPCODE__OP_STATS+1) {
            gettimeofday(&end, NULL);
            time = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec);
            stats_update_operations(stats, 1, time);
        }
        
        // Send message
        result = network_send(client_socket, msg);
        
        if (result == -1) {
            perror("network_main_loop: network_send failed");
            close(client_socket);
            free(args);
            return NULL;
        }
        
        // Free message
        // message_t__free_unpacked(msg, NULL); //está a dar double free()
        msg = NULL; // Reset msg so we can receive a new one in the next iteration
        // Update statistics
    }
    return NULL;
}

/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket){
    if (socket < 0) {
        return -1;
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cv);

    if (close(socket) < 0) {
        perror("network_server_close: close socket failed");
        return -1;
    }

    return 0;
}