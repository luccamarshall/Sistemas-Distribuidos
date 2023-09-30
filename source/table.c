#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "table.h"
#include "table-private.h"

struct table_t
{
    int size;
    int num_entries;
    struct entry_t **entries;
};

struct table_t *table_create(int n)
{

    struct table_t *table = (struct table_t *)malloc(sizeof(struct table_t));
    if (table == NULL)
    {
        return NULL;
    }

    table->size = n;
    table->num_entries = 0;
    table->entries = (struct entry_t **)malloc(sizeof(struct entry_t *) * n);
    if (table->entries == NULL)
    {
        free(table);
        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        table->entries[i] = NULL;
    }

    return table;
}

int table_destroy(struct table_t *table)
{
    if (table == NULL)
    {
        return -1;
    }
    for (int i = 0; i < table->size; i++)
    {
        if (table->entries[i] != NULL)
        {
            entry_destroy(table->entries[i]);
        }
    }
    free(table->entries);
    free(table);
    return 0;
}

int table_put(struct table_t *table, char *key, struct data_t *value)
{
    if (table == NULL || key == NULL || value == NULL)
    {
        return -1;
    }
    int hash = hash_function(key, table->size);
    struct entry_t *entry = entry_create(key, value);
    if (entry == NULL)
    {
        return -1;
    }
    if (table->entries[hash] == NULL)
    {
        table->entries[hash] = entry;
        table->num_entries++;
        return 0;
    }
    else
    {
        entry_destroy(table->entries[hash]);
        table->entries[hash] = entry;
        return 0;
    }
}

struct data_t *table_get(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
    {
        return NULL;
    }
        int hash = hash_function(key, table->size);
        if (table->entries[hash] == NULL)
        {
            return NULL;
        }
        return data_dup(table->entries[hash]->value);
    }

int table_remove(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
    {
        return -1;
    }
    int hash = hash_function(key, table->size);
    if (table->entries[hash] == NULL)
    {
        return 1;
    }
    entry_destroy(table->entries[hash]);
    table->entries[hash] = NULL;
    table->num_entries--;
    return 0;
}

int table_size(struct table_t *table)
{
    if (table == NULL)
    {
        return -1;
    }
    return table->num_entries;
}

char **table_get_keys(struct table_t *table){

}

int table_free_keys(char **keys){
}

hash_function(char *key, int size)
{
    int hash = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash += key[i];
    }
    return hash % size;
}