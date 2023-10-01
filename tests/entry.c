#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "entry.h"

struct entry_t *entry_create(char *key, struct data_t *data){
    if(key == NULL || data == NULL){
        return NULL;
    }
    struct entry_t *entry = (struct entry_t*) malloc(sizeof(struct entry_t));
    if(entry == NULL){
        return NULL;
    }
    entry->key = key;
    entry->value = data;
    return entry;
}

int entry_destroy(struct entry_t *entry){
    if(entry == NULL || entry->key == NULL || entry->value == NULL){
        return -1;
    }
    free(entry->key);
    data_destroy(entry->value);
    free(entry);
    return 0;
}

struct entry_t *entry_dup(struct entry_t *entry){
    if(entry == NULL){
        return NULL;
    }
    struct entry_t *entry_t = (struct entry_t*) malloc(sizeof(struct entry_t));
    if(entry_t == NULL){
        return NULL;
    }
    entry_t->key = strdup(entry->key);
    entry_t->value = data_dup(entry->value);
    return entry_t;
}

int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    if(entry == NULL || new_key == NULL || new_value == NULL){
        return -1;
    }
    free(entry->key);
    data_destroy(entry->value);
    entry->key = strdup(new_key);
    entry->value = data_dup(new_value);
    return 0;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
    if(entry1 == NULL || entry2 == NULL){
        return -2;
    }
    return strcmp(entry1->key, entry2->key);
}