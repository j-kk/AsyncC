//
// Created by Jakub Kowalski on 18/12/2019.
//

#ifndef ASYNCC_FUTURE_H
#define ASYNCC_FUTURE_H

#include <pthread.h>
#include "threadpool.h"

typedef struct callable {
    void *(*function)(void *, size_t, size_t *);

    void *arg;
    size_t argsz;
} callable_t;

typedef struct future {
    bool isdone;
    pthread_mutex_t *done;
    pthread_cond_t *waiting;
    void *mem;
    size_t retsize;
} future_t;

int async(thread_pool_t *pool, future_t *future, callable_t callable);

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *));

void *await(future_t *future);

#endif //ASYNCC_FUTURE_H
