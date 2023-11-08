#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "message-private.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

int write_all(int socket, void *buf, size_t count) {
    int bytes_sent = 0;
    int bytes_left = count;

    while (bytes_sent < count) {
        int n = send(socket, buf + bytes_sent, bytes_left, 0);
        if (n == -1) {
            close(socket);
            return -1;
        }

        bytes_sent += n;
        bytes_left -= n;
    }

    return bytes_sent;
}


int read_all(int socket, void *buf, size_t count) {
    int bytes_read = 0;
    int bytes_left = count;

    while (bytes_read < count) {
        int n = read(socket, buf + bytes_read, bytes_left);
        if (n == -1) {
            close(socket);
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        bytes_read += n;
        bytes_left -= n;
    }

    return bytes_read;
}

