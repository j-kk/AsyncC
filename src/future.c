//
// Created by Jakub Kowalski on 18/12/2019.
//

#include <stdlib.h>
#include "err.h"
#include "future.h"
#include "pthread_macros.h"


pthread_mutex_t globFutureListMutex;
elem_t *fCondList = NULL;
elem_t *fMutexList = NULL;

static void initlib() __attribute__((constructor));

static void initlib() {
    int err;
    MUTEX_INIT_S(&globFutureListMutex)
}

static void exitlib() __attribute__((destructor));

static void exitlib() {
    int err;
    MUTEX_DESTROY_S(&globFutureListMutex)
    pthread_cond_t *cond;
    while ((cond = pop(&fCondList)) != NULL) {
        COND_DESTROY_S(cond)
        free(cond);
    }
    pthread_mutex_t *mutex;
    while ((mutex = pop(&fMutexList)) != NULL) {
        MUTEX_DESTROY_S(mutex)
        free(mutex);
    }
}

typedef struct {
    thread_pool_t *pool;
    future_t *future;
    future_t *from;

    void *(*function)(void *, size_t, size_t *);
}  __attribute__((packed)) mapWorkerArgsT;

void worker(void *args, size_t argSize  __attribute__((unused))) {
    int err;

    callable_t task = *(callable_t *) args;
    future_t *future = *(future_t **) (args + sizeof(callable_t));

    LOCK_S(future->done)

    future->mem = (task.function)(task.arg, task.argsz, &future->retsize);
    future->isdone = true;

    free(args);
    UNLOCK_S(future->done)
    SIGNAL_S(future->waiting)
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    int err;
    void *args;
    //init future
    MALLOC(future->done, sizeof(pthread_mutex_t))
    MALLOC(future->waiting, sizeof(pthread_cond_t))

    MUTEX_INIT(future->done)
    COND_INIT(future->waiting)

    future->isdone = false;
    future->mem = NULL;
    future->retsize = 0;

    LOCK(&globFutureListMutex)

    TRY(add_elem(&fMutexList, future->done), "add_elem")
    TRY(add_elem(&fCondList, future->waiting), "add_elem")

    UNLOCK(&globFutureListMutex)

    MALLOC(args, sizeof(callable_t) + sizeof(future_t *))

    *(callable_t *) (args) = callable;
    *(future_t **) (args + sizeof(callable_t)) = future;

    runnable_t runnable = {
            .function = &worker,
            .arg = args,
            .argsz = sizeof(callable_t) + sizeof(future_t *)
    };
    TRY(defer(pool, runnable), "defer")

    return 0;
}

void *await(future_t *future) {
    int err;
    if (!future->isdone) {
        LOCK_S(future->done)
        while (!future->isdone)
            WAIT_S(future->waiting, future->done)
        UNLOCK_S(future->done)
    }
    return future->mem;
}

void map_worker(void *args, size_t args_size __attribute__((unused))) {
    mapWorkerArgsT *mapWorkerArgs = (mapWorkerArgsT *) args;

    callable_t new_call = {
            .function = mapWorkerArgs->function,
            .arg = await(mapWorkerArgs->from),
            .argsz = mapWorkerArgs->from->retsize
    };

    void *worker_args;
    MALLOC_S(worker_args, sizeof(callable_t) + sizeof(future_t *));

    *(callable_t *) (worker_args) = new_call;
    *(future_t **) (worker_args + sizeof(callable_t)) = mapWorkerArgs->future;

    worker(worker_args, sizeof(callable_t) + sizeof(future_t *));

    free(args);
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    int err;
    if (future == from) //todo
        return -1;

    //init future
    MALLOC(future->done, sizeof(pthread_mutex_t))
    MALLOC(future->waiting, sizeof(pthread_cond_t))

    MUTEX_INIT(future->done)
    COND_INIT(future->waiting)

    future->isdone = false;
    future->mem = NULL;
    future->retsize = 0;

    LOCK(&globFutureListMutex)
    TRY(add_elem(&fMutexList, future->done), "add_elem")
    TRY(add_elem(&fCondList, future->waiting), "add_elem")
    UNLOCK(&globFutureListMutex)

    mapWorkerArgsT *mapWorkerArgs;
    MALLOC(mapWorkerArgs, sizeof(mapWorkerArgsT))

    mapWorkerArgs->pool = pool;
    mapWorkerArgs->from = from;
    mapWorkerArgs->future = future;
    mapWorkerArgs->function = function;

    runnable_t runnable = {
            .function = &map_worker,
            .arg = mapWorkerArgs,
            .argsz = sizeof(mapWorkerArgsT)
    };

    TRY(defer(pool, runnable), "defer")
    return 0;
}
