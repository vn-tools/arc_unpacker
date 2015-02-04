#ifndef COLLECTIONS_PAIR_H
#define COLLECTIONS_PAIR_H

typedef struct
{
    void *e1;
    void *e2;
} Pair;

Pair *pair_create(void *e1, void *e2);

void pair_destroy(Pair *pair);

#endif
