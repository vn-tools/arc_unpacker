#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_parser.h"
#include "assert_ex.h"
#include "cli_helpers.h"
#include "factory/converter_factory.h"
#include "formats/converter.h"
#include "fs.h"
#include "io.h"
#include "logger.h"
#include "output_files.h"

typedef struct
{
    const char *format;
    const char *output_dir;
    Array *input_paths;
} Options;

typedef struct
{
    char *input_path;
    char *target_path;
} PathInfo;

typedef struct
{
    Options *options;
    ArgParser *arg_parser;
    ConverterFactory *conv_factory;
    const PathInfo *path_info;
} ReadContext;

static void add_output_folder_option(ArgParser *arg_parser, Options *options)
{
    arg_parser_add_help(
        arg_parser,
        "-o, --out=FOLDER",
        "Where to put the output files.");

    if (arg_parser_has_switch(arg_parser, "-o"))
        options->output_dir = arg_parser_get_switch(arg_parser, "-o");
    if (arg_parser_has_switch(arg_parser, "--out"))
        options->output_dir = arg_parser_get_switch(arg_parser, "--out");
}

static bool add_input_paths_option(ArgParser *arg_parser, Options *options)
{
    options->input_paths = array_create();

    const Array *stray = arg_parser_get_stray(arg_parser);
    size_t i, j;
    for (i = 1; i < array_size(stray); i ++)
    {
        const char *path = (const char*)array_get(stray, i);
        if (is_dir(path))
        {
            Array *sub_paths = get_files_recursive(path);
            for (j = 0; j < array_size(sub_paths); j ++)
            {
                PathInfo *pi = (PathInfo*)malloc(sizeof(PathInfo));
                pi->input_path = (char*)array_get(sub_paths, j);
                pi->target_path = strdup(pi->input_path + strlen(path) + 1);
                array_add(options->input_paths, pi);
            }
        }
        else
        {
            PathInfo *pi = (PathInfo*)malloc(sizeof(PathInfo));
            pi->input_path = strdup(path);
            pi->target_path = basename(path);
            array_add(options->input_paths, pi);
        }
    }

    if (array_size(options->input_paths) < 1)
    {
        log_error("Required more arguments.");
        return false;
    }

    return true;
}

static void add_format_option(ArgParser *arg_parser, Options *options)
{
    arg_parser_add_help(
        arg_parser,
        "-f, --fmt=FORMAT",
        "Selects the file format.");

    if (arg_parser_has_switch(arg_parser, "-f"))
        options->format = arg_parser_get_switch(arg_parser, "-f");
    if (arg_parser_has_switch(arg_parser, "--fmt"))
        options->format = arg_parser_get_switch(arg_parser, "--fmt");
}

static void print_help(
    const char *path_to_self,
    ArgParser *arg_parser,
    Options *options,
    Converter *converter)
{
    assert_not_null(path_to_self);
    assert_not_null(arg_parser);
    assert_not_null(options);

    printf(
        "Usage: %s [options] [file_options] input_path [input_path...]",
        path_to_self);

    puts("");
    puts("[options] can be:");
    puts("");
    arg_parser_print_help(arg_parser);
    arg_parser_clear_help(arg_parser);
    puts("");

    if (converter == NULL)
    {
        puts("[file_options] depend on chosen format and are required at runtime.");
        puts("See --help --fmt=FORMAT to get detailed help for given converter.");
    }
    else
    {
        converter_add_cli_help(converter, arg_parser);
        printf("[file_options] specific to %s:\n", options->format);
        puts("");
        arg_parser_print_help(arg_parser);
    }
}

static bool decode(
    Converter *converter,
    ArgParser *arg_parser,
    VirtualFile *file)
{
    converter_parse_cli_options(converter, arg_parser);
    return converter_decode(converter, file);
}

static bool guess_converter_and_decode(
    Options *options,
    ArgParser *arg_parser,
    ConverterFactory *conv_factory,
    VirtualFile *file)
{
    assert_not_null(options);
    assert_not_null(arg_parser);
    assert_not_null(conv_factory);
    assert_not_null(file);

    size_t i;
    bool result = false;
    if (options->format == NULL)
    {
        const Array *format_strings = converter_factory_formats(conv_factory);
        assert_not_null(format_strings);
        for (i = 0; i < array_size(format_strings); i ++)
        {
            const char *format = (const char*)array_get(format_strings, i);
            assert_not_null(format);

            Converter *converter
                = converter_factory_from_string(conv_factory, format);
            assert_not_null(converter);
            log_info("Trying %s...", format);
            result = decode(converter, arg_parser, file);
            converter_destroy(converter);

            if (result)
            {
                log_info("Success - %s decoding finished", format);
                break;
            }
            else
            {
                log_info("%s didn\'t work, trying next format...", format);
            }
        }
        if (!result)
        {
            log_error("Nothing left to try. File not recognized.");
        }
    }
    else
    {
        Converter *converter
            = converter_factory_from_string(conv_factory, options->format);
        assert_not_null(converter);
        result = decode(converter, arg_parser, file);
        if (result)
            log_info("Success - %s decoding finished", options->format);
        else
            log_info("Failure - %s decoding finished", options->format);
        converter_destroy(converter);
    }

    return result;
}

static VirtualFile *read_file(const char *file_path)
{
    IO *input_io = io_create_from_file(file_path, "rb");
    if (!input_io)
    {
        log_warning("File %s does not exist.", file_path);
        return NULL;
    }
    VirtualFile *file = vf_create();
    assert_not_null(file);

    char *tmp = (char*)malloc(io_size(input_io));
    assert_not_null(tmp);
    io_read_string(input_io, tmp, io_size(input_io));
    vf_set_data(file, tmp, io_size(input_io));
    free(tmp);

    io_destroy(input_io);
    return file;
}

static void set_file_path(VirtualFile *file, const char *input_path)
{
    char *output_path = (char*)malloc(strlen(input_path) + 2);
    assert_not_null(output_path);
    strcpy(output_path, input_path);
    strcat(output_path, "~");
    vf_set_name(file, output_path);
    free(output_path);
}

static VirtualFile *read_and_decode(void *_context)
{
    ReadContext *context = (ReadContext*)_context;
    VirtualFile *file = read_file(context->path_info->input_path);
    if (file == NULL)
        return NULL;

    set_file_path(
        file,
        context->options->output_dir == NULL
            ? context->path_info->input_path
            : context->path_info->target_path);

    if (!guess_converter_and_decode(
        context->options,
        context->arg_parser,
        context->conv_factory,
        file))
    {
        vf_destroy(file);
        return NULL;
    }

    return file;
}

static bool run(
    Options *options,
    ArgParser *arg_parser,
    ConverterFactory *conv_factory)
{
    size_t i;
    assert_not_null(options);
    assert_not_null(arg_parser);
    assert_not_null(conv_factory);

    OutputFiles *output_files = output_files_create_hdd(options->output_dir);
    assert_not_null(output_files);

    ReadContext context;
    context.arg_parser = arg_parser;
    context.options = options;
    context.conv_factory = conv_factory;

    bool result = true;
    for (i = 0; i < array_size(options->input_paths); i ++)
    {
        context.path_info = (PathInfo*)array_get(options->input_paths, i);
        result &= output_files_save(output_files, &read_and_decode, &context);
    }
    output_files_destroy(output_files);
    return result;
}

int main(int argc, const char **argv)
{
    int exit_code = 0;
    Options options =
    {
        .format = NULL,
        .output_dir = NULL,
        .input_paths = NULL,
    };

    ConverterFactory *conv_factory = converter_factory_create();
    assert_not_null(conv_factory);
    ArgParser *arg_parser = arg_parser_create();
    assert_not_null(arg_parser);
    arg_parser_parse(arg_parser, argc, argv);

    add_output_folder_option(arg_parser, &options);
    add_format_option(arg_parser, &options);
    cli_add_quiet_option(arg_parser);
    cli_add_help_option(arg_parser);

    if (cli_should_show_help(arg_parser))
    {
        Converter *converter = options.format != NULL
            ? converter_factory_from_string(conv_factory, options.format)
            : NULL;
        print_help(argv[0], arg_parser, &options, converter);
        if (converter != NULL)
            converter_destroy(converter);
    }
    else
    {
        if (!add_input_paths_option(arg_parser, &options))
            exit_code = 1;
        else if (!run(&options, arg_parser, conv_factory))
            exit_code = 1;
    }

    if (options.input_paths != NULL)
    {
        size_t i;
        for (i = 0; i < array_size(options.input_paths); i ++)
        {
            PathInfo *pi = (PathInfo*)array_get(options.input_paths, i);
            free(pi->input_path);
            free(pi->target_path);
            free(pi);
        }
        array_destroy(options.input_paths);
    }
    converter_factory_destroy(conv_factory);
    arg_parser_destroy(arg_parser);
    return exit_code;
}
