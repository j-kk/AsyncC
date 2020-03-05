//
// Created by Jakub Kowalski on 18/12/2019.
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/threadpool.h"

void func() {
    int c = 0;
    while (c < 10000000) {
        c++;
    }
    printf("%d\n", c);
}

void create_runnable(runnable_t *r) {
    r->function = &func;
    r->argsz = 0;
    r->arg = NULL;
}

int main() {

    thread_pool_t t;
    thread_pool_init(&t, 5);
    runnable_t arr[10];
    for (int i = 0; i < 10; ++i) {
        create_runnable(&arr[i]);
        defer(&t, arr[i]);
    }
    sleep(1);
    thread_pool_destroy(&t);
    return 0;
}