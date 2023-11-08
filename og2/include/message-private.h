#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

/* Function to write data to a socket and ensure all data is sent */
int write_all(int socket, void *buf, size_t count);

/* Function to read data from a socket and ensure all data is received */
int read_all(int socket, void *buf, size_t count);

#endif
