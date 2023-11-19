#include "stats.h"
#include <stdlib.h>

struct statistics_t *stats_init() {
    struct statistics_t *stats = malloc(sizeof(struct statistics_t));
    if (stats == NULL) {
        return NULL;
    }

    stats->total_operations = 0;
    stats->total_time = 0;
    stats->connected_clients = 0;

    return stats;
}

void stats_update_operations(struct statistics_t *stats, long operations, long time) {
    if (stats == NULL) {
        return;
    }

    stats->total_operations += operations;
    stats->total_time += time;
}

void stats_update_clients(struct statistics_t *stats, int clients) {
    if (stats == NULL) {
        return;
    }

    stats->connected_clients += clients;
}

struct statistics_t *stats_get(struct statistics_t *stats) {
    return stats;
}

void stats_free(struct statistics_t *stats) {
    if (stats != NULL) {
        free(stats);
    }
}