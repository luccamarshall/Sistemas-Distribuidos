#ifndef _MSG_PRIVATE_H
#define _MSG_PRIVATE_H /* Módulo list */

#include <sys/types.h>

ssize_t read_all(int socket, void *buffer, size_t count);

ssize_t write_all(int socket, const void *buffer, size_t count);

#endif