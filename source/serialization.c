#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>

#include "list-private.h" // Certifique-se de incluir o arquivo de cabeçalho apropriado

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    if (keys == NULL || keys_buf == NULL)
    {
        return -1;
    }

    int nkeys = 0;
    int keys_buf_size = 0;

    // Determine the number of keys (nkeys) and the total size of the buffer
    for (int i = 0; keys[i] != NULL; i++)
    {
        nkeys++;
        keys_buf_size += strlen(keys[i]) + 1; // +1 for the null character
    }

    // Allocate memory for the buffer
    *keys_buf = (char *)malloc(sizeof(int) + keys_buf_size);
    if (*keys_buf == NULL)
    {
        free(*keys_buf);
        return -1; // Memory allocation error
    }

    // Convert nkeys to network byte order (big-endian)
    int nkeys_be = htonl(nkeys);

    // Copy the number of keys (nkeys_be) to the beginning of the buffer
    memcpy(*keys_buf, &nkeys_be, sizeof(int));
    char *current_position = *keys_buf + sizeof(int);
    char *original_position = *keys_buf;

    // Copy each key to the buffer
    for (int i = 0; i < nkeys; i++)
    {
        // Determine the length of the key
        int key_length = strlen(keys[i]) + 1; // +1 for the null character

        // Copy the key to the buffer
        strcpy(current_position, keys[i]);
        current_position += key_length; // Advance the buffer pointer
    }
    *keys_buf = original_position;

    return sizeof(int) + keys_buf_size;
}

char **buffer_to_keyArray(char *keys_buf)
{
    if (keys_buf == NULL)
    {
        // Invalid parameter
        return NULL;
    }

    char **keys = NULL;
    int nkeys = 0;

    // Copy the number of keys (nkeys) from the buffer, ensuring endianness
    memcpy(&nkeys, keys_buf, sizeof(int));
    nkeys = ntohl(nkeys); // Convert from network byte order to host byte order
    keys_buf += sizeof(int); // Advance the buffer pointer

    // Allocate memory for the array of strings
    keys = (char **)malloc((nkeys + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL; // Memory allocation error
    }

    // Initialize the last element of the array as NULL
    keys[nkeys] = NULL;

    // Copy each key from the buffer to the array
    for (int i = 0; i < nkeys; i++)
    {
        // Determine the length of the key
        int key_length = strlen(keys_buf) + 1; // +1 for the null terminator

        // Allocate memory for the key and copy it
        keys[i] = (char *)malloc(key_length);
        if (keys[i] == NULL)
        {
            // Memory allocation error
            // Free previously allocated memory and return NULL
            for (int j = 0; j < i; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }

        strcpy(keys[i], keys_buf); // Copy the key from the buffer to the array
        keys_buf += key_length;    // Advance the buffer pointer
    }

    return keys;
}