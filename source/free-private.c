#include "free-private.h"

int free_rtable_t(rtable_t *rtable){
    if (rtable->server_address != NULL) {
        free(rtable->server_address);
        rtable->server_address = NULL;
    }
    free(rtable);
    rtable = NULL;
    return 0;
}

int free_MessageT(MessageT *message){
    if (message->entry != NULL) {
        free_EntryT(message->entry);
        message->entry = NULL;
    }
    if (message->key != NULL) {
        free(message->key);
        message->key = NULL;
    }
    if (message->keys != NULL) {
        for (size_t i = 0; i < message->n_keys; i++) {
            if (message->keys[i] != NULL) {
                free(message->keys[i]);
                message->keys[i] = NULL;
            }
        }
        free(message->keys);
        message->keys = NULL;
    }
    if (message->entries != NULL) {
        for (size_t i = 0; i < message->n_entries; i++) {
            if (message->entries[i] != NULL) {
                free_EntryT(message->entries[i]);
                message->entries[i] = NULL;
            }
        }
        free(message->entries);
        message->entries = NULL;
    }
    if(message->value.data != NULL){
        free(message->value.data);
        message->value.data = NULL;
    }

    free(message);
    message = NULL;
    return 0;
}

int free_EntryT(EntryT *entry){
    if (entry->key != NULL) {
        free(entry->key);
        entry->key = NULL;
    }
    if (entry->value.data != NULL) {
        free(entry->value.data);
        entry->value.data = NULL;
    }
    free(entry);
    entry = NULL;
    return 0;	
}