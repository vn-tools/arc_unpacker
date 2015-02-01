#ifndef ASSERT_H
#define ASSERT_H
#include <stddef.h>
#include <stdbool.h>

#ifdef ENABLE_ASSERT
    #define assert_equali(a, b) _assert_equali(__FILE__, __LINE__, a, b)
    #define assert_equals(a, b) _assert_equals(__FILE__, __LINE__, a, b)
    #define assert_equalsn(a, b, n) _assert_equalsn(__FILE__, __LINE__, a, b, n)
    #define assert_equalp(a, b) _assert_equalp(__FILE__, __LINE__, a, b)
    #define assert_not_null(a) _assert_not_null(__FILE__, __LINE__, a)
    #define assert_null(a) _assert_null(__FILE__, __LINE__, a)
    #define assert_that(a) _assert_that(__FILE__, __LINE__, a)
#else
    #define assert_equali(a, b) (void)(a);(void)(b)
    #define assert_equals(a, b) (void)(a);(void)(b)
    #define assert_equalsn(a, b, n) (void)(a);(void)(b);(void)(n)
    #define assert_equalp(a, b) (void)(a);(void)(b)
    #define assert_not_null(a) (void)(a)
    #define assert_null(a) (void)(a)
    #define assert_that(a) (void)(a)
#endif

void _assert_equali(const char *file, int line, int expected, int actual);

void _assert_equals(
    const char *file,
    int line,
    const char *expected,
    const char *actual);

void _assert_equalsn(
    const char *file,
    int line,
    const char *expected,
    const char *actual,
    size_t size);

void _assert_equalp(
    const char *file,
    int line,
    const void *expected,
    const void *actual);

void _assert_not_null(const char *file, int line, const void *a);

void _assert_null(const char *file, int line, const void *a);

void _assert_that(const char *file, int line, bool expected);

#endif
