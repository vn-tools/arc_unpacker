#include <cassert>
#include "linked_list.h"

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
    LinkedList *linked_list = new LinkedList;
    assert(linked_list != nullptr);
    linked_list->head = nullptr;
    linked_list->current = nullptr;
    linked_list->tail = nullptr;
    linked_list->size = 0;
    return linked_list;
}

void linked_list_destroy(LinkedList *linked_list)
{
    struct ListItem *li, *old;
    assert(linked_list != nullptr);
    li = linked_list->head;
    while (li != nullptr)
    {
        old = li;
        li = li->next;
        delete old;
    }
    delete linked_list;
}

void linked_list_reset(LinkedList *linked_list)
{
    assert(linked_list != nullptr);
    linked_list->current = linked_list->head;
}

void *linked_list_get(LinkedList *linked_list)
{
    assert(linked_list != nullptr);
    if (linked_list->current == nullptr)
        return nullptr;
    return linked_list->current->data;
}

void linked_list_advance(LinkedList *linked_list)
{
    assert(linked_list != nullptr);
    if (linked_list->current != nullptr)
        linked_list->current = linked_list->current->next;
}

void linked_list_add(LinkedList *linked_list, void *item)
{
    assert(linked_list != nullptr);
    struct ListItem *li = new struct ListItem;
    assert(li != nullptr);
    li->data = item;
    li->next = nullptr;
    if (linked_list->tail != nullptr)
        linked_list->tail->next = li;
    linked_list->tail = li;
    if (linked_list->head == nullptr)
        linked_list->head = li;
    ++ linked_list->size;
}

size_t linked_list_size(LinkedList *linked_list)
{
    assert(linked_list != nullptr);
    return linked_list->size;
}
