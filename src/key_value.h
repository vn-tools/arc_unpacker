#ifndef KEY_VALUE_H
#define KEY_VALUE_H

typedef struct
{
    void *key;
    void *value;
} KeyValue;

KeyValue *key_value_create(void *key, void *value);

void key_value_destroy(KeyValue *const kv);

#endif
