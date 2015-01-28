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

void __assert_equali(char *file, int line, int expected, int actual);

void __assert_equals(
    char *file,
    int line,
    const char *const expected,
    const char *const actual);

void __assert_equalsn(
    char *file,
    int line,
    const char *const expected,
    const char *const actual,
    size_t size);

void __assert_equalp(
    char *file,
    int line,
    const void *const expected,
    const void *const actual);

void __assert_not_null(char *file, int line, const void *const a);

void __assert_null(char *file, int line, const void *const a);

void __assert_that(char *file, int line, bool expected);

#endif
