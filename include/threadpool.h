#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>
#include <stdbool.h>

#include "list.h"

typedef struct runnable {
    void (*function)(void *, size_t);

    void *arg;
    size_t argsz;
} runnable_t;

typedef struct thread_pool {
    pthread_mutex_t mutex;
    pthread_t *threads_arr;
    pthread_cond_t wait_cond;
    elem_t *new_runnables;

    size_t pool_size;
    bool sigint_lock;
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif //_THREADPOOL_H_