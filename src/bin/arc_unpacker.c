#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_parser.h"
#include "assert_ex.h"
#include "bin_helpers.h"
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
} Options;

static char *get_default_output_path(const char *input_path)
{
    char *output_path = (char*)malloc(strlen(input_path) + 2);
    assert_not_null(output_path);
    sprintf(output_path, "%s~", input_path);
    return output_path;
}

static void add_format_option(ArgParser *arg_parser, Options *options)
{
    arg_parser_add_help(
        arg_parser,
        "-f, --fmt=FORMAT",
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
    Options *options,
    Archive *archive)
{
    assert_not_null(path_to_self);
    assert_not_null(arg_parser);
    assert_not_null(options);

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

    if (archive != NULL)
    {
        archive_add_cli_help(archive, arg_parser);
        printf("[arc_options] specific to %s:\n", options->format);
        puts("");
        arg_parser_print_help(arg_parser);
        return;
    }
    puts("[arc_options] depend on each archive and are required at runtime.");
    puts("See --help --fmt=FORMAT to get detailed help for given archive.");
}

static bool unpack(
    Archive *archive, ArgParser *arg_parser, IO *io, OutputFiles *output_files)
{
    io_seek(io, 0);
    archive_parse_cli_options(archive, arg_parser);
    return archive_unpack(archive, io, output_files);
}

static bool guess_archive_and_unpack(
    IO *io,
    OutputFiles *output_files,
    Options *options,
    ArgParser *arg_parser,
    ArchiveFactory *arc_factory)
{
    assert_not_null(io);
    assert_not_null(options);
    assert_not_null(arg_parser);
    assert_not_null(arc_factory);
    assert_not_null(output_files);

    size_t i;
    bool result = false;
    if (options->format == NULL)
    {
        const Array *format_strings = archive_factory_formats(arc_factory);
        assert_not_null(format_strings);

        for (i = 0; i < array_size(format_strings); i ++)
        {
            const char *format = (const char*)array_get(format_strings, i);
            assert_not_null(format);

            Archive *archive = archive_factory_from_string(arc_factory, format);
            assert_not_null(archive);
            log_info("Trying %s...", format);
            result = unpack(archive, arg_parser, io, output_files);
            archive_destroy(archive);

            if (result)
            {
                log_info("%s unpacking finished successfully.", format);
                break;
            }
            else
            {
                log_info("%s didn\'t work, trying next format.", format);
                if (log_enabled(LOG_LEVEL_INFO))
                    puts("");
            }
        }
        if (!result)
            log_error("Nothing left to try. File not recognized.");
    }
    else
    {
        Archive *archive
            = archive_factory_from_string(arc_factory, options->format);
        result = unpack(archive, arg_parser, io, output_files);
        if (result)
            log_info("%s unpacking finished successfully.", options->format);
        else
            log_info("%s unpacking finished with errors.", options->format);
        archive_destroy(archive);
    }
    return result;
}

static bool run(
    Options *options,
    ArgParser *arg_parser,
    ArchiveFactory *arc_factory)
{
    assert_not_null(options);
    assert_not_null(arg_parser);
    assert_not_null(arc_factory);

    OutputFiles *output_files = output_files_create_hdd(options->output_path);
    IO *io = io_create_from_file(options->input_path, "rb");
    if (!io)
        return false;

    bool result = guess_archive_and_unpack(
        io,
        output_files,
        options,
        arg_parser,
        arc_factory);

    io_destroy(io);
    output_files_destroy(output_files);
    return result;
}

int main(int argc, const char **argv)
{
    int exit_code = 0;
    Options options =
    {
        .input_path = NULL,
        .output_path = NULL,
        .format = NULL,
    };

    ArchiveFactory *arc_factory = archive_factory_create();
    assert_not_null(arc_factory);
    ArgParser *arg_parser = arg_parser_create();
    assert_not_null(arg_parser);
    arg_parser_parse(arg_parser, argc, argv);

    add_format_option(arg_parser, &options);
    add_quiet_option(arg_parser);
    add_help_option(arg_parser);

    if (should_show_help(arg_parser))
    {
        Archive *archive = options.format != NULL
            ? archive_factory_from_string( arc_factory, options.format)
            : NULL;
        print_help(argv[0], arg_parser, &options, archive);
        if (archive != NULL)
            archive_destroy(archive);
    }
    else
    {
        add_path_options(arg_parser, &options);
        if (!run(&options, arg_parser, arc_factory))
            exit_code = 1;
    }

    archive_factory_destroy(arc_factory);
    arg_parser_destroy(arg_parser);
    return exit_code;
}
