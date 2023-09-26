#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "list.h"
#include "list-private.h"

struct list_t *list_create() {
  struct list_t *lista = (struct list_t *) malloc(sizeof(struct list_t));

  if (lista == NULL)
    return NULL;
  
  lista->size = 0;
  lista->head = NULL;

  return lista;
}
