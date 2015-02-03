#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_parser.h"
#include "assert_ex.h"
#include "key_value.h"
#include "string_ex.h"

static bool is_alphanumeric(const char *string);

static bool get_switch(const char *argument, char **key, char **value);

static bool get_flag(const char *argument, char **value);

static Array *create_words(const char *sentence);

static bool is_alphanumeric(const char *string)
{
    unsigned int i;
    for (i = 0; i < strlen(string); i ++)
    {
        char c = string[i];
        if (c < '0' && c > '9' && c < 'a' && c > 'a' && c < 'A' && c > 'A')
            return false;
    }
    return true;
}

static bool get_switch(const char *argument, char **key, char **value)
{
    char *value_ptr = NULL;

    assert_not_null(argument);

    *key = NULL;
    *value = NULL;

    if (argument[0] != '-')
        return false;
    while (argument[0] == '-')
        argument ++;

    value_ptr = ((char*)memchr(argument, '=', strlen(argument)));
    if (value_ptr == NULL)
        return false;

    *key = strndup(argument, value_ptr - argument);
    if (!is_alphanumeric(*key))
    {
        free(key);
        return false;
    }
    *value = strdup(value_ptr + 1);
    return true;
}

static bool get_flag(const char *argument, char **value)
{
    assert_not_null(argument);

    *value = NULL;

    if (argument[0] != '-')
        return false;
    while (argument[0] == '-')
        argument ++;

    if (!is_alphanumeric(argument))
        return false;

    *value = strdup(argument);
    return true;
}

static Array *create_words(const char *sentence)
{
    Array *words;
    size_t i = 0;
    const char *word = sentence;
    const char *next_word;

    assert_not_null(sentence);

    words = array_create();
    next_word = strpbrk(word, " ");
    while (next_word != NULL)
    {
        char *new_word = strndup(word, next_word - word);
        array_add(words, new_word);
        word = next_word + 1;
        next_word = strpbrk(word, " ");
        i ++;
    }
    array_add(words, strdup(word));
    return words;
}



struct ArgParser
{
    LinkedList *switches;
    LinkedList *flags;
    Array *stray;
    LinkedList *help_items;
};

ArgParser *arg_parser_create()
{
    ArgParser *arg_parser = (ArgParser*)malloc(sizeof(ArgParser));
    assert_not_null(arg_parser);
    arg_parser->flags = linked_list_create();
    arg_parser->switches = linked_list_create();
    arg_parser->stray = array_create();
    arg_parser->help_items = linked_list_create();
    return arg_parser;
}

void arg_parser_destroy(ArgParser *arg_parser)
{
    size_t i;
    void *item;
    assert_not_null(arg_parser);

    if (arg_parser->switches != NULL)
    {
        linked_list_reset(arg_parser->switches);
        while ((item = linked_list_get(arg_parser->switches)) != NULL)
        {
            linked_list_advance(arg_parser->switches);
            KeyValue *kv = (KeyValue*)item;
            free(kv->key);
            free(kv->value);
            key_value_destroy(kv);
        }
        linked_list_destroy(arg_parser->switches);
    }

    if (arg_parser->flags != NULL)
    {
        linked_list_reset(arg_parser->flags);
        while ((item = linked_list_get(arg_parser->flags)) != NULL)
        {
            linked_list_advance(arg_parser->flags);
            free(item);
        }
        linked_list_destroy(arg_parser->flags);
    }

    if (arg_parser->stray != NULL)
    {
        for (i = 0; i < array_size(arg_parser->stray); i ++)
            free(array_get(arg_parser->stray, i));
        array_destroy(arg_parser->stray);
    }

    if (arg_parser->help_items != NULL)
        linked_list_destroy(arg_parser->help_items);

    free(arg_parser);
}

void arg_parser_parse(ArgParser *arg_parser, int argc, const char **argv)
{
    const char *arg;
    char *key, *value;
    int i;

    if (argc == 0)
        return;

    assert_not_null(argv);

    for (i = 0; i < argc; i ++)
    {
        arg = argv[i];
        assert_not_null(arg);

        key = NULL;
        value = NULL;
        if (get_switch(arg, &key, &value))
        {
            linked_list_add(arg_parser->switches, key_value_create(key, value));
        }
        else if (get_flag(arg, &value))
        {
            linked_list_add(arg_parser->flags, value);
        }
        else
        {
            array_add(arg_parser->stray, strdup(arg));
        }
    }
}

void arg_parser_clear_help(ArgParser *arg_parser)
{
    assert_not_null(arg_parser);

    linked_list_destroy(arg_parser->help_items);
    arg_parser->help_items = linked_list_create();
}

void arg_parser_add_help(
    ArgParser *arg_parser,
    const char *invocation,
    const char *description)
{
    assert_not_null(arg_parser);
    assert_not_null(invocation);
    assert_not_null(description);

    KeyValue *kv = key_value_create((void*)invocation, (void*)description);
    linked_list_add(arg_parser->help_items, kv);
}

bool arg_parser_has_switch(ArgParser *arg_parser, const char *key)
{
    KeyValue *kv;
    assert_not_null(arg_parser);
    assert_not_null(key);
    while (key[0] == '-')
        key ++;
    linked_list_reset(arg_parser->switches);
    while ((kv = (KeyValue*)linked_list_get(arg_parser->switches)) != NULL)
    {
        linked_list_advance(arg_parser->switches);
        if (strcmp((char*)kv->key, key) == 0)
            return true;
    }
    return false;
}

char *arg_parser_get_switch(ArgParser *arg_parser, const char *key)
{
    KeyValue *kv;

    assert_not_null(arg_parser);
    assert_not_null(key);

    while (key[0] == '-')
        key ++;

    linked_list_reset(arg_parser->switches);
    while ((kv = (KeyValue*)linked_list_get(arg_parser->switches)) != NULL)
    {
        linked_list_advance(arg_parser->switches);
        if (strcmp((char*)kv->key, key) == 0)
            return (char*)kv->value;
    }
    return NULL;
}

bool arg_parser_has_flag(ArgParser *arg_parser, const char *flag)
{
    void *item;

    assert_not_null(arg_parser);
    assert_not_null(flag);

    while (flag[0] == '-')
        flag ++;

    linked_list_reset(arg_parser->flags);
    while ((item = linked_list_get(arg_parser->flags)) != NULL)
    {
        linked_list_advance(arg_parser->flags);
        if (strcmp((char*)item, flag) == 0)
            return true;
    }
    return false;
}

Array *arg_parser_get_stray(ArgParser *arg_parser)
{
    assert_not_null(arg_parser);
    return arg_parser->stray;
}

void arg_parser_print_help(ArgParser *arg_parser)
{
    const size_t max_invocation_length = 25;
    const size_t max_line_length = 78;
    const char *invocation, *description;
    char *word;
    void *item;
    size_t i, j;
    size_t tmp_length;
    KeyValue *kv;
    bool first_word;

    assert_not_null(arg_parser);

    if (linked_list_size(arg_parser->help_items) == 0)
    {
        puts("No additional switches are available.");
        return;
    }

    linked_list_reset(arg_parser->help_items);
    while ((item = linked_list_get(arg_parser->help_items)) != NULL)
    {
        linked_list_advance(arg_parser->help_items);

        kv = (KeyValue*) item;
        invocation = (const char*)kv->key;
        description = (const char*)kv->value;

        tmp_length = strlen(invocation);
        if (strlen(invocation) >= 2 && strncmp(invocation, "--", 2) == 0)
        {
            printf("    ");
            tmp_length += 4;
        }
        printf(invocation);

        for (; tmp_length < max_invocation_length; tmp_length ++)
            printf(" ");

        //word wrap
        Array *words = create_words(description);
        first_word = true;
        for (i = 0; i < array_size(words); i ++)
        {
            word = (char*)array_get(words, i);
            tmp_length += strlen(word);
            if (!first_word && tmp_length > max_line_length)
            {
                printf("\n");
                for (j = 0; j < max_invocation_length; j ++)
                    printf(" ");
                tmp_length = max_invocation_length;
            }
            printf("%s ", word);
            free(word);
            first_word = false;
        }
        printf("\n");
        array_destroy(words);
    }
}
