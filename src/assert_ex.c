#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "logger.h"

static void fail(const char *file, int line, const char *format, ...);

static void fail(const char *file, int line, const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    log_error("%s in %s:%d", buffer, file, line);
    exit(1);
}



void _assert_equali(const char *file, int line, int expected, int actual)
{
    if (actual != expected)
        fail(file, line, "Fatal: expected %d, got %d", expected, actual);
}

void _assert_equals(
    const char *file,
    int line,
    const char *expected,
    const char *actual)
{
    if (expected == NULL && actual == NULL)
        return;

    if (expected == NULL || actual == NULL || strcmp(actual, expected) != 0)
        fail(file, line, "Fatal: expected %s, got %s", expected, actual);
}

void _assert_equalsn(
    const char *file,
    int line,
    const char *expected,
    const char *actual,
    size_t size)
{
    if (expected == NULL && actual == NULL)
        return;

    if (expected == NULL || actual == NULL)
        fail(file, line, "Fatal: expected %s, got %s", expected, actual);

    size_t i;
    for (i = 0; i < size; i ++)
    {
        if (actual[i] != expected[i])
        {
            fail(
                file,
                line,
                "Fatal: expected %c (%02x), got %c (%02x) at position %d",
                expected[i],
                (unsigned)expected[i],
                actual[i],
                (unsigned)actual[i],
                i);
        }
    }
}

void _assert_equalp(
    const char *file,
    int line,
    const void *expected,
    const void *actual)
{
    if (actual != expected)
        fail(file, line, "Fatal: expected %p, got %p", expected, actual);
}

void _assert_null(const char *file, int line, const void *data)
{
    if (data != NULL)
        fail(file, line, "Fatal: expected NULL, got %p", data);
}

void _assert_not_null(const char *file, int line, const void *data)
{
    if (data == NULL)
        fail(file, line, "Fatal: expected not NULL, got NULL.");
}

void _assert_that(const char *file, int line, bool expected)
{
    if (!expected)
        fail(file, line, "Fatal: expected true, got false.");
}
