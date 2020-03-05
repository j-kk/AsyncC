//
// Created by Jakub Kowalski on 28/12/2019.
//
#include <stdlib.h>
#include "pthread_macros.h"
#include "list.h"
#include "err.h"

int add_elem(elem_t **list, void *elem) {
    if (*list == NULL) {
        MALLOC(*list, sizeof(elem_t))
        (*list)->next_el = NULL;
        (*list)->var = elem;
        return 0;
    } else {
        return add_elem(&(*list)->next_el, elem);
    }
}

int del_elem(elem_t **list, void *elem) {
    if (*list == NULL) {
        return -1;
    } else {
        if ((*list)->var == elem) {
            elem_t *to_remove = *list;
            *list = (*list)->next_el;
            free(to_remove);
            return 0;
        } else {
            return del_elem(&(*list)->next_el, elem);
        }
    }
}

void *pop(elem_t **list) {
    void *to_return = NULL;
    if (*list != NULL) {
        to_return = (*list)->var;
        elem_t *to_remove = *list;
        *list = (*list)->next_el;
        free(to_remove);
    }
    return to_return;
}