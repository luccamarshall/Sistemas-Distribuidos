#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "list-private.h" // Certifique-se de incluir o arquivo de cabeçalho apropriado

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    if (keys == NULL || keys_buf == NULL)
    {
        // Parâmetro inválido
        return -1;
    }

    int nkeys = 0;
    int keys_buf_size = 0;

    // Determine o número de chaves (nkeys) e o tamanho total do buffer
    for (int i = 0; keys[i] != NULL; i++)
    {
        nkeys++;
        keys_buf_size += strlen(keys[i]) + 1; // +1 para o caractere nulo
    }

    // Aloque memória para o buffer
    *keys_buf = (char *) malloc(sizeof(int) + keys_buf_size);
    if (*keys_buf == NULL)
    {
        return -1; // Erro de alocação de memória
    }

    // Copie o número de chaves (nkeys) para o início do buffer
    memcpy(*keys_buf, &nkeys, sizeof(int));
    *keys_buf += sizeof(int); // Avance o ponteiro do buffer

    // Copie cada chave para o buffer
    for (int i = 0; i < nkeys; i++)
    {
        // Determine o comprimento da chave
        int key_length = strlen(keys[i]) + 1; // +1 para o caractere nulo

        // Copie a chave para o buffer
        strcpy(*keys_buf, keys[i]);
        *keys_buf += key_length; // Avance o ponteiro do buffer
    }
    keys_buf -= keys_buf_size + sizeof(int); // Volte o ponteiro do buffer

    return sizeof(int) + keys_buf_size;
}

char **buffer_to_keyArray(char *keys_buf)
{
    if (keys_buf == NULL)
    {
        // Parâmetro inválido
        return NULL;
    }

    char **keys = NULL;
    int nkeys = 0;

    // Copie o número de chaves (nkeys) do início do buffer
    memcpy(&nkeys, keys_buf, sizeof(int));
    keys_buf += sizeof(int); // Avance o ponteiro do buffer

    // Aloque memória para o array de strings
    keys = (char **)malloc((nkeys + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL; // Erro de alocação de memória
    }

    // Inicialize o último elemento do array como NULL
    keys[nkeys] = NULL;

    // Copie cada chave do buffer para o array
    for (int i = 0; i < nkeys; i++)
    {
        // Determine o comprimento da chave
        int key_length = strlen(keys_buf) + 1; // +1 para o caractere nulo

        // Aloque memória para a chave e copie-a
        keys[i] = (char *)malloc(key_length);
        if (keys[i] == NULL)
        {
            // Erro de alocação de memória
            // Libere a memória alocada até agora e retorne NULL
            for (int j = 0; j < i; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }

        strcpy(keys[i], keys_buf); // Copie a chave do buffer para o array
        keys_buf += key_length;    // Avance o ponteiro do buffer
    }

    return keys;
}