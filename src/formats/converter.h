#ifndef FORMATS_CONVERTER_H
#define FORMATS_CONVERTER_H
#include <stdbool.h>
#include "arg_parser.h"
#include "virtual_file.h"

typedef struct Converter
{
    void *data;

    void (*add_cli_help)(struct Converter *, ArgParser *);
    void (*parse_cli_options)(struct Converter *, ArgParser *);
    bool (*decode)(struct Converter *, VirtualFile *);
    void (*cleanup)(struct Converter *);
} Converter;

Converter *converter_create();

void converter_destroy(Converter *converter);

void converter_parse_cli_options();

void converter_add_cli_help(Converter *converter, ArgParser *arg_parser);

bool converter_decode(Converter *converter, VirtualFile *target_file);

#endif
