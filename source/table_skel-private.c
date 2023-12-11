#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "table_skel-private.h"
#include "list.h"
#include "entry.h"

struct entry_t *table_get_n_entry(struct list_t *list, int n) {
    if (list == NULL || n < 0 || n >= list->size) {
        return NULL;
    }

    struct node_t *current = list->head;
    for (int i = 0; i < n; i++) {
        current = current->next;
    }

    return current->entry;
}

