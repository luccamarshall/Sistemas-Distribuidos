#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "list-private.h"

struct node_t
{
    struct entry_t *entry;
    struct node_t *next;
};

struct list_t
{
    int size;
    struct node_t *head;
};