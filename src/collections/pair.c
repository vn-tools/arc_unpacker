#include <assert.h>
#include <stdlib.h>
#include "pair.h"

Pair *pair_create(void *e1, void *e2)
{
    Pair *pair = (Pair*)malloc(sizeof(Pair));
    assert(pair != NULL);
    pair->e1 = e1;
    pair->e2 = e2;
    return pair;
}

void pair_destroy(Pair *pair)
{
    free(pair);
}
