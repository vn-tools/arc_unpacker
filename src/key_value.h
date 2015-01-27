#ifndef KEY_VALUE_H
#define KEY_VALUE_H

typedef struct
{
    const void *key;
    const void *value;
} KeyValue;

KeyValue *key_value_create(const void *key, const void *value);

void key_value_destroy(KeyValue *kv);

#endif
