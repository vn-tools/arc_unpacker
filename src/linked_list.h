#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stddef.h>

typedef struct LinkedList LinkedList;

LinkedList *linked_list_create();

void linked_list_destroy(LinkedList *linked_list);

void linked_list_reset(LinkedList *linked_list);

void *linked_list_get(LinkedList *linked_list);

void linked_list_advance(LinkedList *linked_list);

void linked_list_add(LinkedList *linked_list, void *item);

size_t linked_list_size(LinkedList *linked_list);

#endif
