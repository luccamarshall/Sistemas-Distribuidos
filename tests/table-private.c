#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "list-private.h"

struct table_t;

int hash_code(char *key, int n)
{
    int i, sum = 0;
    for (i = 0; key[i] != '\0'; i++)
    {
        sum += key[i];
    }
    return sum % n;
}