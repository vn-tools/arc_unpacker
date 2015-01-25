#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

void assert_equali(int expected, int actual)
{
	if (actual != expected)
	{
		fprintf(stderr, "Fatal: expected %d, got %d.\n", expected, actual);
		exit(1);
	}
}

void assert_equalp(const void *const expected, const void *const actual)
{
	if (actual != expected)
	{
		fprintf(stderr, "Fatal: expected %p, got %p.\n", expected, actual);
		exit(1);
	}
}

void assert_null(const void *const data)
{
	if (data != NULL)
	{
		fprintf(stderr, "Fatal: expected NULL, got %p.\n", data);
		exit(1);
	}
}

void assert_not_null(const void *const data)
{
	if (data == NULL)
	{
		fprintf(stderr, "Fatal: expected not NULL, got NULL.\n");
		exit(1);
	}
}

void assert_that(bool expected)
{
	if (!expected)
	{
		fprintf(stderr, "Fatal: expected true, got false.\n");
		exit(1);
	}
}
