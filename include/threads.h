#ifndef _THREADS_H
#define _THREADS_H

#include "sdmessage.pb-c.h"

typedef struct {
  pthread_mutex_t lock;
  pthread_cond_t cv;
  int writing;
  int num_readers;
} SynchronizedTable;


void threads_init(SynchronizedTable *st);

void threads_destroy(SynchronizedTable *st);

void threads_lock(SynchronizedTable *st, MessageT *msg);

void threads_unlock(SynchronizedTable *st, MessageT *msg);


#endif