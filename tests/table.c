#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "table-private.h"
#include "table.h"
#include "entry.h"
#include "data.h"

struct table_t *table_create(int n)
{

    struct table_t *table = (struct table_t *)malloc(sizeof(struct table_t));
    if (table == NULL)
    {
        return NULL;
    }

    table->size = n;
    table->num_entries = 0;
    table->lists = (struct list_t **)malloc(sizeof(struct entry_t *) * n);
    if (table->lists == NULL)
    {
        free(table);
        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        table->lists[i] = NULL;
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
        list_destroy(table->lists[i]);
    }
    free(table);
    return 0;
}

int table_put(struct table_t *table, char *key, struct data_t *value)
{
    if (table == NULL || key == NULL || value == NULL) {
        return -1;
    }

    int hash = hash_code(key, table->size);
    
    if (table->lists[hash] == NULL) {
        table->lists[hash] = list_create();
    }
    struct entry_t *entry = entry_create(key, value);

    return list_add(table->lists[hash], entry);
   
}

struct data_t *table_get(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL) {
        return NULL;
    }

    int index = hash_code(key, table->size);
    if (table->lists[index] == NULL) {
        return NULL;
    }
    struct entry_t *entry = list_get(table->lists[index], key);
    if (entry == NULL) {
        return NULL;
    }
    return data_dup(entry->value);
}

int table_remove(struct table_t *table, char *key)
{

    if (table == NULL || key == NULL) {
        return -1;
    }

    int hash = hash_code(key, table->size);

    if (table->lists[hash] == NULL) {
        return 1;
    }

    if (list_remove(table->lists[hash], key) == 0) {
        table->num_entries--;
        return 0;
    }
    return 1;
}

int table_size(struct table_t *table)
{
    if (table == NULL)
    {
        return -1;
    }
    int num_entries = 0;

    for(int i = 0; i < table->size; i++) {
        if (table->lists[i] != NULL) {
            num_entries += table->lists[i]->size;
        }
    }
    return num_entries;
}

char **table_get_keys(struct table_t *table) {

    if (table == NULL) {
        return NULL;
    }
    char **keys = (char **)malloc(sizeof(char *) * table->num_entries);
    if (keys == NULL) {
        return NULL;
    }
    int i = 0;
    for (int j = 0; j < table->size; j++) {
        if (table->lists[j] != NULL) {
            struct node_t *node = table->lists[j]->head;
            while (node != NULL) {
                keys[i] = strdup(node->entry->key);
                i++;
                node = node->next;
            }
        }
    }
    keys[i] = NULL;
    return keys;

}

int table_free_keys(char **keys) {

    if (keys == NULL)
    {
        return -1;
    }
    for (int i = 0; keys[i] != NULL; i++)
    {
        free(keys[i]);
    }
    free(keys);
    return 0;
}