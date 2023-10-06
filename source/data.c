#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "data.h"

struct data_t *data_create(int size, void *data){
    struct data_t *data_t = (struct data_t*) malloc(sizeof(struct data_t));
    if(data_t == NULL || size <= 0 || data == NULL) {
        free(data_t);
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
    if(data == NULL || data->data == NULL || data->datasize <= 0) {
        return NULL;
    }
    struct data_t *newdata = (struct data_t*) malloc(sizeof(struct data_t));
    if(newdata == NULL){
        free(newdata);
        return NULL;
    }
    newdata->datasize = data->datasize;
    newdata->data = malloc(data->datasize);
    if(newdata->data == NULL) {
        free(newdata);
        free(newdata->data);
        return NULL;
    }
    memcpy(newdata->data, data->data, data->datasize);
    return newdata;
}

int data_replace(struct data_t *data, int new_size, void *new_data) {
    if(data == NULL || new_size <= 0 || new_data == NULL){
        return -1;
    }
    free(data->data);
    data->datasize = new_size;
    data->data = new_data;
    return 0;
}