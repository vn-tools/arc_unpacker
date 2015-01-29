#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_parser.h"
#include "assert.h"
#include "cli_helpers.h"
#include "factory/archive_factory.h"
#include "formats/archive.h"
#include "logger.h"
#include "output_files.h"
#include "virtual_file.h"

typedef struct
{
    const char *format;
    const char *input_path;
    const char *output_path;
    Archive *archive;
} Options;

static char *get_default_output_path(const char *input_path)
{
    char *output_path = strdup(input_path);
    assert_not_null(output_path);
    output_path = (char*)realloc(output_path, strlen(output_path) + 1);
    assert_not_null(output_path);
    strcat(output_path, "~");
    return output_path;
}

static void add_format_option(ArgParser *arg_parser, Options *options)
{
    arg_parser_add_help(
        arg_parser,
        "-f, --fmt FORMAT",
        "Selects the archive format.");

    if (arg_parser_has_switch(arg_parser, "-f"))
        options->format = arg_parser_get_switch(arg_parser, "-f");
    if (arg_parser_has_switch(arg_parser, "--fmt"))
        options->format = arg_parser_get_switch(arg_parser, "--fmt");
}

static void add_path_options(ArgParser *arg_parser, Options *options)
{
    Array *stray = arg_parser_get_stray(arg_parser);
    if (array_size(stray) < 2)
    {
        errno = EINVAL;
        log_error("Required more arguments.");
        exit(1);
    }

    options->input_path = (const char*)array_get(stray, 1);
    options->output_path = array_size(stray) == 2
        ? get_default_output_path((const char*)array_get(stray, 1))
        : (const char*)array_get(stray, 2);
}

static void print_help(
    const char *path_to_self,
    ArgParser *arg_parser,
    Options *options)
{
    printf("Usage: %s [options] [arc_options] input_path [output_path]\n",
        path_to_self);

    puts("");
    puts("Unless output path is provided, the script is going to use path");
    puts("followed with a tilde (~).");
    puts("");
    puts("[options] can be:");
    puts("");

    arg_parser_print_help(arg_parser);
    arg_parser_clear_help(arg_parser);
    puts("");

    if (options->archive == NULL)
    {
        puts("[arc_options] depend on each archive and are required at runtime.");
        puts("See --help --fmt FORMAT to get detailed help for given archive.");
    }
    else
    {
        archive_add_cli_help(options->archive, arg_parser);
        printf("[arc_options] specific to %s:\n", options->format);
        puts("");
        arg_parser_print_help(arg_parser);
    }
}

static bool unpack(
    Archive *archive, ArgParser *arg_parser, IO *io, OutputFiles *output_files)
{
    bool result;
    archive_parse_cli_options(archive, arg_parser);
    result = archive_unpack(archive, io, output_files);
    archive_destroy(archive);
    return result;
}

static bool run(
    Options *options,
    ArgParser *arg_parser,
    ArchiveFactory *arc_factory)
{
    OutputFiles *output_files;
    IO *io;
    const Array *format_strings;
    const char *format;
    bool result = false;
    size_t i;

    assert_not_null(options);

    output_files = output_files_create(options->output_path);
    io = io_create_from_file(options->input_path, "rb");
    if (!io)
        return false;

    if (options->archive == NULL)
    {
        format_strings = archive_factory_formats(arc_factory);
        for (i = 0; i < array_size(format_strings); i ++)
        {
            format = (const char*)array_get(format_strings, i);
            options->archive = archive_factory_from_string(arc_factory, format);
            assert_not_null(options->archive);
            log_info("Trying %s", format);
            result = unpack(options->archive, arg_parser, io, output_files);
            if (result)
                break;
            else
                log_info(NULL);
        }
    }
    else
    {
        result = unpack(options->archive, arg_parser, io, output_files);
    }

    output_files_destroy(output_files);
    io_destroy(io);
    return result;

}

int main(int argc, const char **argv)
{
    Options options =
    {
        .input_path = NULL,
        .output_path = NULL,
        .format = NULL,
    };

    ArchiveFactory *arc_factory = archive_factory_create();
    ArgParser *arg_parser = arg_parser_create();
    assert_not_null(arg_parser);
    arg_parser_parse(arg_parser, argc, argv);

    add_format_option(arg_parser, &options);
    if (options.format != NULL)
    {
        options.archive = archive_factory_from_string(
            arc_factory,
            options.format);
        if (options.archive == NULL)
        {
            return 1;
        }
    }
    cli_add_quiet_option(arg_parser);
    cli_add_help_option(arg_parser);

    if (cli_should_show_help(arg_parser))
    {
        print_help(argv[0], arg_parser, &options);
    }
    else
    {
        add_path_options(arg_parser, &options);
        if (!run(&options, arg_parser, arc_factory))
            return 1;
    }

    archive_factory_destroy(arc_factory);
    arg_parser_destroy(arg_parser);
    return 0;
}
