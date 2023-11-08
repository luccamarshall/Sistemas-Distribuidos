#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>

#include "list-private.h"

/*
Nº de grupo: 14
Bruno Sarkar 56899
Lucca Marshall 58233
Tiago Rocha 58242
*/

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    if (keys == NULL || keys_buf == NULL)
    {
        return -1;
    }

    int nkeys = 0;
    int keys_buf_size = 0;

   
    for (int i = 0; keys[i] != NULL; i++)
    {
        nkeys++;
        keys_buf_size += strlen(keys[i]) + 1; 
    }

    *keys_buf = (char *)malloc(sizeof(int) + keys_buf_size);
    if (*keys_buf == NULL)
    {
        free(*keys_buf);
        return -1;
    }

    int nkeys_be = htonl(nkeys);

    memcpy(*keys_buf, &nkeys_be, sizeof(int));
    char *current_position = *keys_buf + sizeof(int);
    char *original_position = *keys_buf;

    for (int i = 0; i < nkeys; i++)
    {
        int key_length = strlen(keys[i]) + 1;

        strcpy(current_position, keys[i]);
        current_position += key_length;
    }
    *keys_buf = original_position;

    return sizeof(int) + keys_buf_size;
}

char **buffer_to_keyArray(char *keys_buf)
{
    if (keys_buf == NULL)
    {
        return NULL;
    }

    char **keys = NULL;
    int nkeys = 0;

    memcpy(&nkeys, keys_buf, sizeof(int));
    nkeys = ntohl(nkeys);
    keys_buf += sizeof(int);

    keys = (char **)malloc((nkeys + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL;
    }

    keys[nkeys] = NULL;

    for (int i = 0; i < nkeys; i++)
    {
        int key_length = strlen(keys_buf) + 1; // +1 for the null terminator

        keys[i] = (char *)malloc(key_length);
        if (keys[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        strcpy(keys[i], keys_buf);
        keys_buf += key_length;
    }

    return keys;
}