#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "list.h"

struct table_t
{
    int size;
    int num_entries;
    struct list_t **lists;
};

/* 
 * Função que calcula o índice da lista a partir da chave
 */
int hash_code(char *key, int size);

#endif