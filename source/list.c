#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "list.h"
#include "list-private.h"

struct list_t;

struct list_t *list_create()
{
    struct list_t *list = (struct list_t *)malloc(sizeof(struct list_t));
    if (list == NULL)
    {
        return NULL;
    }
    list->size = 0;
    list->head = NULL;
    list->head->entry = NULL;
    list->head->next = NULL;
    return list;
}

int list_destroy(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }
    struct node_t *node = list->head;
    struct node_t *next;
    while (node != NULL)
    {
        next = node->next;
        entry_destroy(node->entry);
        free(node);
        node = next;
    }
    free(list);
    return 0;
}

int list_add(struct list_t *list, struct entry_t *entry)
{
    if (list == NULL || entry == NULL)
    {
        return -1;
    }
    struct node_t *node = list->head;
    struct node_t *prev = NULL;
    while (node != NULL)
    {
        if (entry_compare(node->entry, entry) == 0)
        {
            if (prev == NULL)
            {
                list->head = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            entry_destroy(node->entry);
            free(node);
            list->size--;
            break;
        }
        prev = node;
        node = node->next;
    }
    node = (struct node_t *)malloc(sizeof(struct node_t));
    if (node == NULL)
    {
        return -1;
    }
    node->entry = entry_dup(entry);
    if (node->entry == NULL)
    {
        free(node);
        return -1;
    }
    node->next = NULL;
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
    return 0;
}

int list_remove(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
    {
        return -1;
    }
    struct node_t *node = list->head;
    struct node_t *prev = NULL;
    while (node != NULL)
    {
        if (strcmp(node->entry->key, key) == 0)
        {
            if (prev == NULL)
            {
                list->head = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            entry_destroy(node->entry);
            free(node);
            list->size--;
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return 1;
}

struct entry_t *list_get(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
    {
        return NULL;
    }
    struct node_t *node = list->head;
    while (node != NULL)
    {
        if (strcmp(node->entry->key, key) == 0)
        {
            return node->entry;
        }
        node = node->next;
    }
    return NULL;
}

int list_size(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }
    return list->size;
}

char **list_get_keys(struct list_t *list)
{
    if (list == NULL)
    {
        return NULL;
    }
    char **keys = (char **)malloc((list->size + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL;
    }
    struct node_t *node = list->head;
    int i = 0;
    while (node != NULL)
    {
        keys[i] = strdup(node->entry->key);
        node = node->next;
        i++;
    }
    keys[i] = NULL;
    return keys;
}
