#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "data.h"

struct data_t *data_create(int size, void *data){
    struct data_t *data_t = (struct data_t*) malloc(sizeof(struct data_t));
    if(data_t == NULL){
        return NULL;
    }
    data_t->datasize = size;
    data_t->data = data;
    return data_t;
}

int data_destroy(struct data_t *data){
    if(data == NULL){
        return -1;
    }
    free(data->data);
    free(data);
    return 0;
}

struct data_t *data_dup(struct data_t *data){
    if(data == NULL){
        return NULL;
    }
    struct data_t *data_t = (struct data_t*) malloc(sizeof(struct data_t));
    if(data_t == NULL){
        return NULL;
    }
    data_t->datasize = data->datasize;
    data_t->data = malloc(data->datasize);
    if(data_t->data == NULL){
        free(data_t);
        return NULL;
    }
    memcpy(data_t->data, data->data, data->datasize);
    return data_t;
}