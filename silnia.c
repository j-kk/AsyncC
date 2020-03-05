//
// Created by Jakub Kowalski on 31/12/2019.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "err.h"
#include "future.h"

thread_pool_t pool;

void *silnia(void *args, size_t arg_size  __attribute__((unused)), size_t *ret_size  __attribute__((unused))) {

    long *ret = malloc(2 * sizeof(long));
    if (ret == NULL) {
        fatal("Malloc");
        exit(1);
    }
    long nth = ((long *) args)[0];
    long ilo = ((long *) args)[1];

    if (nth > 2)
        free(args);

    ret[0] = nth + 3;
    ret[1] = nth == 0 ? 1 : ilo * nth;
    return ret;
}

int main() {
    int err;
    long var;

    if ((err = thread_pool_init(&pool, 1)) != 0) //todo 3
        return syserr(err, "thread pool init");

    scanf("%ld", &var);

    callable_t calls[3];
    long args_val[3][2];
    future_t futures[var + 1];
    for (long i = 0; i < 3; ++i) {
        args_val[i][0] = i;
        args_val[i][1] = 1;

        calls[i].function = &silnia;
        calls[i].arg = &args_val[i];
        calls[i].argsz = sizeof(long) * 2;

        if ((err = async(&pool, &futures[i], calls[i])) != 0)
            return syserr(err, "async err");
    }

    for (long i = 3; i <= var; ++i) {
        if ((err = map(&pool, &futures[i], &futures[i - 3], &silnia)) != 0)
            return syserr(err, "map");
    }
    long final = 1;
    for (long i = var - 2 > 0 ? var - 2 : 0; i <= var; ++i) {
        void *ret;
        if ((ret = await(&futures[i])) == NULL)
            return fatal("await");

        final *= ((long *) ret)[1];
        free(ret);
    }

    printf("%ld", final);

    thread_pool_destroy(&pool);

    return 0;
}