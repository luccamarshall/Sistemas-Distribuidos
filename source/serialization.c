#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "list-private.h" // Certifique-se de incluir o arquivo de cabeçalho apropriado

int keyArray_to_buffer(char **keys, char **keys_buf)
{
    if (keys == NULL || keys_buf == NULL)
    {
        // Parâmetros inválidos
        return -1;
    }

    int nkeys = 0;       // Número de chaves
    int buffer_size = 0; // Tamanho total do buffer
    char *buffer = NULL; // Buffer para armazenar os dados serializados
    char **current_key = keys;

    // Calcule o número de chaves e o tamanho total do buffer
    while (*current_key != NULL)
    {
        nkeys++;
        buffer_size += strlen(*current_key) + 1; // +1 para o caractere nulo
        current_key++;
    }

    // Aloque memória para o buffer
    buffer = (char *)malloc(sizeof(int) + buffer_size);
    if (buffer == NULL)
    {
        return -1; // Erro de alocação de memória
    }

    // Copie o número de chaves (nkeys) para o início do buffer
    memcpy(buffer, &nkeys, sizeof(int));
    buffer += sizeof(int); // Avance o ponteiro do buffer

    // Copie cada chave para o buffer
    current_key = keys;
    while (*current_key != NULL)
    {
        int key_length = strlen(*current_key) + 1; // +1 para o caractere nulo
        memcpy(buffer, *current_key, key_length);
        buffer += key_length; // Avance o ponteiro do buffer
        current_key++;
    }

    *keys_buf = buffer - buffer_size; // Restaure o ponteiro original do buffer

    return sizeof(int) + buffer_size; // Tamanho total do buffer
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