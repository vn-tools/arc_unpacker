#include <stdlib.h>
#include "assert.h"
#include "key_value.h"

KeyValue *key_value_create(void *key, void *value)
{
    KeyValue *kv = malloc(sizeof(KeyValue));
    assert_not_null(kv);
    kv->key = key;
    kv->value = value;
    return kv;
}

void key_value_destroy(KeyValue *const kv)
{
    free(kv);
}
