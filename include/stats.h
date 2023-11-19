#ifndef STATS_H
#define STATS_H

#include <sys/time.h>

struct statistics_t {
    long total_operations; // Total number of operations executed on the server's table
    long total_time; // Total accumulated time spent on executing operations on the table in microseconds
    int connected_clients; // Number of clients currently connected to the server
};

struct statistics_t *stats_init();
void stats_update_operations(struct statistics_t *stats, long operations, long time);
void stats_update_clients(struct statistics_t *stats, int clients);
struct statistics_t *stats_get(struct statistics_t *stats);
void stats_free(struct statistics_t *stats);

#endif // STATS_H