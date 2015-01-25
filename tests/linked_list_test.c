#include "linked_list.h"
#include "assert.h"

void test_getting_empty_without_resetting()
{
    LinkedList *list = linked_list_create();
    assert_null(linked_list_get(list));
    linked_list_destroy(list);
}

void test_getting_empty_with_resetting()
{
    LinkedList *list = linked_list_create();
    linked_list_reset(list);
    assert_null(linked_list_get(list));
    linked_list_destroy(list);
}

void test_getting_nonempty_without_resetting()
{
    int item = 5;
    LinkedList *list = linked_list_create();
    linked_list_add(list, &item);
    assert_null(linked_list_get(list));
    linked_list_destroy(list);
}

void test_getting_nonempty_with_resetting()
{
    int item = 5;
    LinkedList *list = linked_list_create();
    linked_list_add(list, &item);
    linked_list_reset(list);
    assert_equalp(&item, linked_list_get(list));
    linked_list_destroy(list);
}

void test_getting_twice_without_advance()
{
    int item1 = 5;
    int item2 = 5;
    LinkedList *list = linked_list_create();
    linked_list_add(list, &item1);
    linked_list_add(list, &item2);
    linked_list_reset(list);
    assert_equalp(&item1, linked_list_get(list));
    assert_equalp(&item1, linked_list_get(list));
    linked_list_destroy(list);
}

void test_getting_twice_with_advance()
{
    int item1 = 5;
    int item2 = 5;
    LinkedList *list = linked_list_create();
    linked_list_add(list, &item1);
    linked_list_add(list, &item2);
    linked_list_reset(list);
    assert_equalp(&item1, linked_list_get(list));
    linked_list_advance(list);
    assert_equalp(&item2, linked_list_get(list));
    linked_list_destroy(list);
}

void test_size()
{
    int item1 = 5;
    int item2 = 5;
    LinkedList *list = linked_list_create();
    assert_equali(0, linked_list_size(list));
    linked_list_add(list, &item1);
    assert_equali(1, linked_list_size(list));
    linked_list_add(list, &item2);
    assert_equali(2, linked_list_size(list));
    linked_list_destroy(list);
}

int main(void)
{
    test_getting_empty_without_resetting();
    test_getting_empty_with_resetting();
    test_getting_nonempty_without_resetting();
    test_getting_nonempty_with_resetting();
    test_getting_twice_without_advance();
    test_getting_twice_with_advance();
    test_size();
    return 0;
}
