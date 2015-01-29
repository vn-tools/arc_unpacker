#ifndef ASSERT_H
#define ASSERT_H
#include <stddef.h>
#include <stdbool.h>

#ifdef ENABLE_ASSERT
	#define assert_equali(a, b) __assert_equali(__FILE__, __LINE__, a, b)
	#define assert_equals(a, b) __assert_equals(__FILE__, __LINE__, a, b)
	#define assert_equalsn(a, b, n) __assert_equalsn(__FILE__, __LINE__, a, b, n)
	#define assert_equalp(a, b) __assert_equalp(__FILE__, __LINE__, a, b)
	#define assert_not_null(a) __assert_not_null( __FILE__, __LINE__, a)
	#define assert_null(a) __assert_null(__FILE__, __LINE__, a)
	#define assert_that(a) __assert_that(__FILE__, __LINE__, a)
#else
	#define assert_equali(a, b) ((void)0)
	#define assert_equals(a, b) ((void)0)
	#define assert_equalsn(a, b, n) ((void)0)
	#define assert_equalp(a, b) ((void)0)
	#define assert_not_null(a) ((void)0)
	#define assert_null(a) ((void)0)
	#define assert_that(a) ((void)0)
#endif

void __assert_equali(const char *file, int line, int expected, int actual);

void __assert_equals(
    const char *file,
    int line,
    const char *expected,
    const char *actual);

void __assert_equalsn(
    const char *file,
    int line,
    const char *expected,
    const char *actual,
    size_t size);

void __assert_equalp(
    const char *file,
    int line,
    const void *expected,
    const void *actual);

void __assert_not_null(const char *file, int line, const void *a);

void __assert_null(const char *file, int line, const void *a);

void __assert_that(const char *file, int line, bool expected);

#endif
