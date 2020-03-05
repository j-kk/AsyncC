//
// Created by Jakub Kowalski on 06/01/2020.
//

#ifndef ASYNC_PTHREAD_MACROS_H
#define ASYNC_PTHREAD_MACROS_H

#define TRY(ret, error) \
if (ret != 0) \
return syserr(ret, error);

#define TRY_S(ret, error) \
if (ret != 0) \
syserr(ret, error);

#define LOCK(mutex) \
if ((err = pthread_mutex_lock(mutex)) != 0) \
return syserr(err, "lock");

#define UNLOCK(mutex) \
if ((err = pthread_mutex_unlock(mutex)) != 0) \
return syserr(err, "unlock");

#define LOCK_S(mutex) \
if ((err = pthread_mutex_lock(mutex)) != 0) \
syserr(err, "lock");

#define UNLOCK_S(mutex) \
if ((err = pthread_mutex_unlock(mutex)) != 0) \
syserr(err, "unlock");

#define MUTEX_INIT(mutex) \
if ((err = pthread_mutex_init(mutex, 0)) != 0) \
return syserr(err, "mutex init");

#define MUTEX_INIT_S(mutex) \
if ((err = pthread_mutex_init(mutex, 0)) != 0) \
syserr(err, "mutex init");

#define MUTEX_DESTROY_S(mutex) \
if ((err = pthread_mutex_destroy(mutex)) != 0) \
syserr(err, "mutex destroy");

#define COND_INIT(cond) \
if ((err = pthread_cond_init(cond, 0)) != 0) \
return syserr(err, "cond init");

#define COND_DESTROY_S(cond) \
if ((err = pthread_cond_destroy(cond)) != 0) \
syserr(err, "cond destroy");

#define SIGNAL(cond) \
if ((err = pthread_cond_signal(cond)) != 0) \
return syserr(err, "signal");

#define SIGNAL_S(cond) \
if ((err = pthread_cond_signal(cond)) != 0) \
syserr(err, "signal");

#define BROADCAST_S(cond) \
if ((err = pthread_cond_broadcast(cond)) != 0) \
syserr(err, "signal");

#define WAIT_S(cond, mutex) \
if ((err = pthread_cond_wait(cond, mutex)) != 0) \
syserr(err, "wait");

#define MALLOC(ptr, size) \
if ((ptr = malloc(size)) == NULL) \
return fatal("malloc");

#define MALLOC_S(ptr, size) \
if ((ptr = malloc(size)) == NULL) \
fatal("malloc");

#endif //ASYNC_PTHREAD_MACROS_H
