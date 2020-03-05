//
// Created by Jakub Kowalski on 25/12/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <future.h>
#include <threadpool.h>

int msleep(long msec) {
    struct timespec ts;
    int res;

    if (msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void *calc(void *args, size_t arg_size, size_t *retsize) {
    if (arg_size != 2 * sizeof(long))
        exit(1);
    long v = *(long *) args;
    long s = *(long *) (args + sizeof(long));
    msleep(s);
    *retsize = sizeof(long);
    void *ptr = malloc(sizeof(long));
    *(long *) ptr = v;
    return ptr;
}

int main() {
    int W, K;
    scanf("%d", &W);
    scanf("%d", &K);

    future_t MF[W][K];
    callable_t MC[W][K];
    long matrix[W * K][2];
    long *matrixptr = *matrix;
    thread_pool_t pool;

    if (thread_pool_init(&pool, 4) != 0)
        exit(1);

    for (int w = 0; w < W; ++w) {
        for (int k = 0; k < K; ++k) {
            scanf("%ld %ld", matrixptr, matrixptr + 1);

            MC[w][k].function = &calc;
            MC[w][k].arg = matrixptr;
            MC[w][k].argsz = 2 * sizeof(long);

            *(long *) MC[w][k].arg = *matrixptr;
            *(long *) (MC[w][k].arg + sizeof(long)) = *(matrixptr + 1);

            if (async(&pool, &MF[w][k], MC[w][k]) != 0)
                exit(1);
            matrixptr += 2;
        }
    }

    for (int w = 0; w < W; ++w) {
        long suma = 0;
        for (int k = 0; k < K; ++k) {
            long *res = await(&MF[w][k]);
            suma += *res;
            free(res);
        }
        printf("%ld\n", suma);
    }

    thread_pool_destroy(&pool);

    return 0;
}
