#ifndef ARG_PARSER_H
#define ARG_PARSER_H
#include <stdbool.h>
#include "collections/array.h"
#include "collections/linked_list.h"

typedef struct ArgParser ArgParser;

ArgParser *arg_parser_create();

void arg_parser_destroy(ArgParser *arg_parser);

void arg_parser_clear_help(ArgParser *arg_parser);

void arg_parser_parse(ArgParser *arg_parser, int argc, const char **argv);

bool arg_parser_has_flag(ArgParser *arg_parser, const char *argument);

bool arg_parser_has_switch(ArgParser *arg_parser, const char *key);

char *arg_parser_get_switch(ArgParser *arg_parser, const char *key);

Array *arg_parser_get_stray(ArgParser *arg_parser);

void arg_parser_add_help(
    ArgParser *arg_parser,
    const char *invocation,
    const char *description);

void arg_parser_print_help(ArgParser *arg_parser);

#endif
