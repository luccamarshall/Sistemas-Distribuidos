#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "list.h"
#include "list-private.h"

struct list_t;

struct node_t;


struct list_t *list_create()
{
    struct list_t *new_list = (struct list_t *)malloc(sizeof(struct list_t));
    if (new_list == NULL)
    {
        return NULL;
    }
    new_list->size = 0;
    new_list->head = NULL;

    return new_list;
}

int list_destroy(struct list_t *list)
{
    if (list == NULL)
    {
        return -1;
    }
    struct node_t *current = list->head;

    while (current != NULL)
    {
        struct node_t *next = current->next;
        if (current->entry != NULL)
        {
            free(current->entry);
        }
        free(current);
        current = next;
    }
    free(list);

    return 0; // Sucesso
}

int list_add(struct list_t *list, struct entry_t *entry){
    if (list == NULL || entry == NULL)
    {
        return -1;
    }
    struct node_t *current = list->head;
    struct node_t *prev = NULL;

    while (current != NULL)
    {
        int compare_result = entry_compare(current->entry, entry);

        if (compare_result == 0)
        {

            struct entry_t *old_entry = current->entry;
            current->entry = entry;
            free_entry(old_entry); 
            return 1;            
        }
        else if (compare_result > 0)
        {
            struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
            if (new_node == NULL)
            {
                return -1; 
            }
            new_node->entry = entry;
            new_node->next = current;

            if (prev == NULL)
            {
                list->head = new_node;
            }
            else
            {
                prev->next = new_node;
            }
            list->size++;
            return 0; 
        }

        prev = current;
        current = current->next;
    }

    struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
    if (new_node == NULL)
    {
        return -1; 
    }

    new_node->entry = entry;
    new_node->next = NULL;

    if (prev == NULL)
    {
     
        list->head = new_node;
    }
    else
    {
        prev->next = new_node;
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

    struct node_t *current = list->head;
    struct node_t *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->entry->key, key) == 0)
        {
            if (prev == NULL)
            {
                list->head = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free_entry(current->entry);
            free(current);
            list->size--;

            return 0; 
        }
        prev = current;
        current = current->next;
    }
    return 1;
}

struct entry_t *list_get(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
    {
        return NULL;
    }

    struct node_t *current = list->head;

    while (current != NULL)
    {
        if (strcmp(current->entry->key, key) == 0)
        {
            return current->entry;
        }

        current = current->next;
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

    char **keys_array = NULL;
    int num_keys = list->size;
    int i = 0;

    if (num_keys <= 0)
    {
        keys_array = (char **)malloc(sizeof(char *));
        if (keys_array == NULL)
        {
            return NULL; 
        }
        keys_array[0] = NULL;
        return keys_array;
    }
    
    keys_array = (char **)malloc((num_keys + 1) * sizeof(char *));
    if (keys_array == NULL)
    {
        return NULL; 
    }

    struct node_t *current = list->head;

    while (current != NULL)
    {
        keys_array[i] = strdup(current->entry->key);
        if (keys_array[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(keys_array[j]);
            }
            free(keys_array);
            return NULL;
        }

        current = current->next;
        i++;
    }
    keys_array[num_keys] = NULL;

    return keys_array;
}

int list_free_keys(char **keys)
{
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