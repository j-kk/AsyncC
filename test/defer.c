#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#include "minunit.h"
#include "threadpool.h"

int tests_run = 0;

#define NROUNDS 10

static void pong_ping(void *args, size_t argsz __attribute__((unused))) {
    sem_t *ping = args;
    sem_t *pong = (sem_t *) args + 1;

    for (int i = 0; i < NROUNDS; ++i) {
        sem_wait(ping);
        sem_post(pong);
    }

    sem_destroy(ping);
}


static char *ping_pong() {

    thread_pool_t pool;
    mu_assert("def", thread_pool_init(&pool, 1) == 0);

    sem_t *pingpong = malloc(sizeof(sem_t) * 2);
    sem_t *ping = pingpong;
    sem_t *pong = pingpong + 1;

    sem_init(ping, 0, 0);
    sem_init(pong, 0, 0);

    mu_assert("def", defer(&pool, (runnable_t) {.function = pong_ping,
            .arg = pingpong,
            .argsz = sizeof(sem_t) * 2}) == 0);

    for (int i = 0; i < NROUNDS; ++i) {
        sem_post(ping);
        sem_wait(pong);
    }

    sem_destroy(pong);
    free(pingpong);

    thread_pool_destroy(&pool);
    return 0;
}

void sleepm(void *x, size_t s) {
    int v = *(int *) x;
    printf("in: %d\n", v);
    sleep(v);
    printf("out: %d\n", v);
}

void sleepm2(void *x, size_t s) {
    char v = *(char *) x;
    char id = *(char *) (x + 1);
    printf("in: %d\n", id);
    sleep(v);
    printf("out: %d\n", id);
}

static char *defer_after_destroy() {

    printf("delafterdestroy\n");
    thread_pool_t pool;
    thread_pool_init(&pool, 1);
    unsigned int t = 1;
    mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm,
            .arg = &t,
            .argsz = sizeof(int)}) == 0);

    thread_pool_destroy(&pool);
    mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm,
            .arg = &t,
            .argsz = sizeof(int)}) != 0);

    return 0;
}

static char *over() {
    printf("over\n");
    thread_pool_t pool;
    thread_pool_init(&pool, 2);
    unsigned int t2 = 2;
    unsigned int t3 = 3;
    unsigned int t4 = 4;

    mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm,
            .arg = &t2,
            .argsz = sizeof(int)}) == 0);

    mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm,
            .arg = &t3,
            .argsz = sizeof(int)}) == 0);

    mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm,
            .arg = &t4,
            .argsz = sizeof(int)}) == 0);

    thread_pool_destroy(&pool);
    return 0;
}

static char *sigtest() {
    printf("SIGTEST\n");
    thread_pool_t pool;
    thread_pool_init(&pool, 2);
    unsigned int t2 = 2;
    unsigned int t3 = 3;
    unsigned int t4 = 4;
    char ar2[20][2];
    for (int i = 0; i < 20; ++i) {
        printf("i: %d\n", i);
        char *ar = ar2[i];
        ar[0] = 1;
        ar[1] = (char) i;
        if (i <= 5) {
            mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm2,
                    .arg = ar,
                    .argsz = sizeof(char) * 2}) == 0);
        } else {
            mu_assert("def", defer(&pool, (runnable_t) {.function = sleepm2,
                    .arg = ar,
                    .argsz = sizeof(char) * 2}) != 0);
        }
        if (i == 5) {
            kill(getpid(), SIGINT);
            printf("%u", SA_RESETHAND);
            printf("Kill!");
        }

    }

//    thread_pool_destroy(&pool);
    return 0;
}

static char *all_tests() {
    mu_run_test(ping_pong);
    mu_run_test(defer_after_destroy);
    mu_run_test(over);
    mu_run_test(sigtest);
    return 0;
}

int main() {
    char *result = all_tests();
    if (result != 0) {
        printf(__FILE__ ": %s\n", result);
    } else {
        printf(__FILE__ ": ALL TESTS PASSED\n");
    }
    printf(__FILE__ "Tests run: %d\n", tests_run);

    return result != 0;
}
