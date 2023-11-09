#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "message-private.h"

ssize_t read_all(int socket, void *buffer, size_t count) {
    size_t total_bytes_read = 0;

    while (total_bytes_read < count) {
        ssize_t bytes_read = read(socket, buffer + total_bytes_read, count - total_bytes_read);

        if (bytes_read == -1) {
            return -1;
        } else if (bytes_read == 0) {
            // Conexão encerrada pelo outro lado
            return total_bytes_read;
        }

        total_bytes_read += bytes_read;
    }

    return total_bytes_read;
}

// Função para garantir a escrita completa de dados no socket
ssize_t write_all(int socket, const void *buffer, size_t count) {
    size_t total_bytes_written = 0;

    while (total_bytes_written < count) {
        ssize_t bytes_written = write(socket, buffer + total_bytes_written, count - total_bytes_written);

        if (bytes_written == -1) {
            return -1;
        }

        total_bytes_written += bytes_written;
    }

    return total_bytes_written;
}