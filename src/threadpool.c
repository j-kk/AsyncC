//
// Created by Jakub Kowalski on 18/12/2019.
//

#include <pthread.h>
#include <stdlib.h>
#include <signal.h>

#include "threadpool.h"
#include "pthread_macros.h"
#include "err.h"
#include "list.h"

pthread_mutex_t glob_pool_list_mutex;
elem_t *pool_list = NULL;

void clean_pool(int sig __attribute__((unused))) {
    int err;
    thread_pool_t *pool;
    elem_t *tmp_pool_list = NULL;
    LOCK_S(&glob_pool_list_mutex)
    while ((pool = pop(&pool_list)) != NULL) {
        pool->sigint_lock = true;
        TRY_S(add_elem(&tmp_pool_list, pool), "add_elem")
    }
    while ((pool = pop(&tmp_pool_list)) != NULL) {
        thread_pool_destroy(pool);
    }
    UNLOCK_S(&glob_pool_list_mutex)
}

static void initlib() __attribute__((constructor));

static void initlib() {
    int err;
    MUTEX_INIT_S(&glob_pool_list_mutex)

    sigset_t sigint_mask;
    sigemptyset(&sigint_mask);
    sigaddset(&sigint_mask, SIGINT);

    struct sigaction action;

    action.sa_handler = clean_pool;
    action.sa_mask = sigint_mask;
    action.sa_flags = 0;
    TRY_S(sigaction(SIGINT, &action, 0), "sigaction")
}

static void exitlib() __attribute__((destructor));

static void exitlib() {
    int err;
    MUTEX_DESTROY_S(&glob_pool_list_mutex)
}

void worker_job(thread_pool_t *pool) {
    int err;
    runnable_t *my_task = NULL;

    while (true) {
        LOCK_S(&pool->mutex)

        while ((my_task = pop(&pool->new_runnables)) == NULL && pool->pool_size > 0)
            WAIT_S(&pool->wait_cond, &pool->mutex)

        UNLOCK_S(&pool->mutex)

        if (my_task != NULL) {
            (my_task->function)(my_task->arg, my_task->argsz);
            free(my_task);
        } else {
            return;
        }
    }
}

int thread_pool_init(thread_pool_t *pool, size_t pool_size) {
    int err;
    pthread_attr_t attr;

    pool->sigint_lock = false;
    pool->pool_size = pool_size;
    pool->new_runnables = NULL;

    TRY(pthread_attr_init(&attr), "attr init")
    TRY(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE), "set detach state")

    MUTEX_INIT(&pool->mutex)
    COND_INIT(&pool->wait_cond)

    pool->threads_arr = malloc(pool->pool_size * sizeof(pthread_t *));
    for (size_t t = 0; t < pool->pool_size; ++t) {
        TRY(pthread_create(&pool->threads_arr[t], &attr, (void *(*)(void *)) worker_job, pool), "create thread")
    }
    TRY(pthread_attr_destroy(&attr), "destroy attr")

    LOCK(&glob_pool_list_mutex)

    TRY(add_elem(&pool_list, pool), "add_elem")
    UNLOCK(&glob_pool_list_mutex)

    return 0;
}

void thread_pool_destroy(thread_pool_t *pool) {
    int err;
    size_t totPoolSize = pool->pool_size;

    LOCK_S(&pool->mutex)
    pool->pool_size = 0;
    if (!pool->sigint_lock) {
        pool->sigint_lock = true;
        LOCK_S(&glob_pool_list_mutex)
        TRY_S(del_elem(&pool_list, pool), "del_elem")
        UNLOCK_S(&glob_pool_list_mutex)
    }
    UNLOCK_S(&pool->mutex) //todo after BROADCAST?

    BROADCAST_S(&pool->wait_cond)

    for (size_t t = 0; t < totPoolSize; ++t) {
        TRY_S(pthread_join(pool->threads_arr[t], NULL), "join workers")
    }
    MUTEX_DESTROY_S(&pool->mutex)
    COND_DESTROY_S(&pool->wait_cond)

    free(pool->threads_arr);
}

int defer(thread_pool_t *pool, runnable_t runnable) {
    int err;

    if (pool->sigint_lock)
        return -1;

    LOCK(&pool->mutex)

    if (pool->pool_size == 0)
        return -1;

    runnable_t *new_runnable;
    MALLOC(new_runnable, sizeof(runnable_t))

    *new_runnable = runnable;

    TRY(add_elem(&pool->new_runnables, new_runnable), "add_elem")

    UNLOCK(&pool->mutex)
    SIGNAL(&pool->wait_cond)
    return 0;
}