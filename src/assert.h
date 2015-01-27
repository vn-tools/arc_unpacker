#ifndef ASSERT_H
#define ASSERT_H
#include <stddef.h>
#include <stdbool.h>

void assert_equali(int expected, int actual);

void assert_equals(const char *const expected, const char *const actual);

void assert_equalsn(
    const char *const expected,
    const char *const actual,
    size_t size);

void assert_equalp(const void *const expected, const void *const actual);

void assert_null(const void *const a);

void assert_not_null(const void *const a);

void assert_that(bool expected);

#endif
