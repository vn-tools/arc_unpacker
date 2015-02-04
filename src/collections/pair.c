#include <stdlib.h>
#include "assert_ex.h"
#include "pair.h"

Pair *pair_create(void *e1, void *e2)
{
    Pair *pair = (Pair*)malloc(sizeof(Pair));
    assert_not_null(pair);
    pair->e1 = e1;
    pair->e2 = e2;
    return pair;
}

void pair_destroy(Pair *pair)
{
    free(pair);
}
