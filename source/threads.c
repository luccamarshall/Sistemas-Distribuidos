#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "threads.h"

typedef struct SynchronizedTable;

void threads_init(SynchronizedTable *st) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv, NULL);
    st->writing = 0;
    st->num_readers = 0;
    st->table = 0;
}

void threads_destroy(SynchronizedTable *st) {
    pthread_mutex_destroy(&st->lock);
    pthread_cond_destroy(&st->cv);
}

void threads_lock(SynchronizedTable *st, MessageT *msg) {
    pthread_mutex_lock(&st->lock);

    if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT || msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        while(st->num_readers > 0) {
            pthread_cond_wait(&st->cv, &st->lock);
        }
        st->writing = 1;
    } else {
        while(st->writing) {
            pthread_cond_wait(&st->cv, &st->lock);
        }
        st->num_readers++;
    }

    pthread_mutex_unlock(&st->lock);
}

void threads_unlock(SynchronizedTable *st, MessageT *msg) {
    pthread_mutex_lock(&st->lock);

    if (msg->opcode == MESSAGE_T__OPCODE__OP_PUT + 1 || msg->opcode == MESSAGE_T__OPCODE__OP_DEL + 1) {
        st->writing = 0;
        pthread_cond_broadcast(&st->cv);
    } else {
        st->num_readers--;
        if (st->num_readers == 0) {
            pthread_cond_signal(&st->cv);
        }
    }

    pthread_mutex_unlock(&st->lock);
}
