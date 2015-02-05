#ifndef FORMATS_ARCHIVE_H
#define FORMATS_ARCHIVE_H
#include <stdbool.h>
#include "arg_parser.h"
#include "io.h"
#include "output_files.h"

typedef struct Archive
{
    void *data;

    void (*add_cli_help)(struct Archive *, ArgParser&);
    void (*parse_cli_options)(struct Archive *, ArgParser&);
    bool (*unpack)(struct Archive *, IO *, OutputFiles *);
    void (*cleanup)(struct Archive *);
} Archive;

Archive *archive_create();

void archive_destroy(Archive *archive);

void archive_parse_cli_options(Archive *archive, ArgParser &arg_parser);

void archive_add_cli_help(Archive *archive, ArgParser &arg_parser);

bool archive_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files);

#endif
