#include <stdlib.h>
#include "linked_list.h"
#include "assert.h"

struct ListItem
{
    void *data;
    struct ListItem *next;
};

struct LinkedList
{
    size_t size;
    struct ListItem *head;
    struct ListItem *current;
    struct ListItem *tail;
};

LinkedList *linked_list_create()
{
    LinkedList *linked_list = malloc(sizeof(LinkedList));
    linked_list->head = NULL;
    linked_list->current = NULL;
    linked_list->tail = NULL;
    return linked_list;
}

void linked_list_destroy(LinkedList *linked_list)
{
    if (linked_list == NULL)
        return;

    struct ListItem *li = linked_list->head;
    struct ListItem *old;
    while (li != NULL)
    {
        old = li;
        li = li->next;
        free(old);
    }

    free(linked_list);
}

void linked_list_reset(LinkedList *linked_list)
{
    linked_list->current = linked_list->head;
}

void *linked_list_get(LinkedList *linked_list)
{
    assert_not_null(linked_list);
    if (linked_list->current == NULL)
        return NULL;
    return linked_list->current->data;
}

void linked_list_advance(LinkedList *linked_list)
{
    assert_not_null(linked_list);
    if (linked_list->current != NULL)
        linked_list->current = linked_list->current->next;
}

void linked_list_add(LinkedList *linked_list, void *item)
{
    struct ListItem *li = malloc(sizeof(li));
    li->data = item;
    li->next = NULL;
    if (linked_list->tail != NULL)
        linked_list->tail->next = li;
    linked_list->tail = li;
    if (linked_list->head == NULL)
        linked_list->head = li;
    ++ linked_list->size;
}

size_t linked_list_size(LinkedList *linked_list)
{
    return linked_list->size;
}
