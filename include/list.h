//
// Created by Jakub Kowalski on 28/12/2019.
//

#ifndef ASYNCC_LIST_H
#define ASYNCC_LIST_H

typedef struct elem {
    struct elem *next_el;
    void *var;
} elem_t;

int add_elem(elem_t **list, void *elem);

int del_elem(elem_t **list, void *elem);

void *pop(elem_t **list);

#endif //ASYNCC_LIST_H
