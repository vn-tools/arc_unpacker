#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "string_ext.h"

void test_trim_right_nothing_to_trim()
{
    char *target = strdup("abc");
    trim_right(target, " ");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_one()
{
    char *target = strdup("abc ");
    trim_right(target, " ");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_two()
{
    char *target = strdup("abc  ");
    trim_right(target, " ");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_different_characters()
{
    char *target = strdup("abc \t ");
    trim_right(target, " \t");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_to_empty_string()
{
    char *target = strdup(" \t ");
    trim_right(target, " \t");
    assert_equals("", target);
    free(target);
}

int main(void)
{
    test_trim_right_nothing_to_trim();
    test_trim_right_trim_one();
    test_trim_right_trim_two();
    test_trim_right_trim_different_characters();
    test_trim_right_trim_to_empty_string();
    return 0;
}
