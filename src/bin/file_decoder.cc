#include <cassert>
#include <cstdio>
#include <cstring>
#include "arg_parser.h"
#include "bin_helpers.h"
#include "factory/converter_factory.h"
#include "formats/converter.h"
#include "fs.h"
#include "io.h"
#include "logger.h"
#include "output_files.h"
#include "string_ex.h"

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
    ArgParser &arg_parser;
    ConverterFactory *conv_factory;
    const PathInfo *path_info;
} ReadContext;

static void add_output_folder_option(ArgParser &arg_parser, Options *options)
{
    arg_parser.add_help(
        "-o, --out=FOLDER",
        "Where to put the output files.");

    if (arg_parser.has_switch("-o"))
        options->output_dir = arg_parser.get_switch("-o").c_str();
    if (arg_parser.has_switch("--out"))
        options->output_dir = arg_parser.get_switch("--out").c_str();
}

static bool add_input_paths_option(ArgParser &arg_parser, Options *options)
{
    options->input_paths = array_create();

    const std::vector<std::string> stray = arg_parser.get_stray();
    for (size_t i = 1; i < stray.size(); i ++)
    {
        const char *path = stray[i].c_str();
        if (is_dir(path))
        {
            Array *sub_paths = get_files_recursive(path);
            for (size_t j = 0; j < array_size(sub_paths); j ++)
            {
                PathInfo *pi = new PathInfo;
                pi->input_path = (char*)array_get(sub_paths, j);
                pi->target_path = strdup(pi->input_path + strlen(path) + 1);
                array_add(options->input_paths, pi);
            }
        }
        else
        {
            PathInfo *pi = new PathInfo;
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

static void add_format_option(ArgParser &arg_parser, Options *options)
{
    arg_parser.add_help(
        "-f, --fmt=FORMAT",
        "Selects the file format.");

    if (arg_parser.has_switch("-f"))
        options->format = arg_parser.get_switch("-f").c_str();
    if (arg_parser.has_switch("--fmt"))
        options->format = arg_parser.get_switch("--fmt").c_str();
}

static void print_help(
    const char *path_to_self,
    ArgParser &arg_parser,
    Options *options,
    Converter *converter)
{
    assert(path_to_self != nullptr);
    assert(options != nullptr);

    printf(
        "Usage: %s [options] [file_options] input_path [input_path...]",
        path_to_self);

    puts("");
    puts("[options] can be:");
    puts("");
    arg_parser.print_help();
    arg_parser.clear_help();
    puts("");

    if (converter != nullptr)
    {
        converter_add_cli_help(converter, arg_parser);
        printf("[file_options] specific to %s:\n", options->format);
        puts("");
        arg_parser.print_help();
        return;
    }
    puts("[file_options] depend on chosen format and are required at runtime.");
    puts("See --help --fmt=FORMAT to get detailed help for given converter.");
}

static bool decode(
    Converter *converter,
    ArgParser &arg_parser,
    VirtualFile *file)
{
    converter_parse_cli_options(converter, arg_parser);
    return converter_decode(converter, file);
}

static bool guess_converter_and_decode(
    Options *options,
    ArgParser &arg_parser,
    ConverterFactory *conv_factory,
    VirtualFile *file)
{
    assert(options != nullptr);
    assert(conv_factory != nullptr);
    assert(file != nullptr);

    size_t i;
    bool result = false;
    if (options->format == nullptr)
    {
        const Array *format_strings = converter_factory_formats(conv_factory);
        assert(format_strings != nullptr);
        for (i = 0; i < array_size(format_strings); i ++)
        {
            const char *format = (const char*)array_get(format_strings, i);
            assert(format != nullptr);

            Converter *converter
                = converter_factory_from_string(conv_factory, format);
            assert(converter != nullptr);
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
        if (converter != nullptr)
        {
            result = decode(converter, arg_parser, file);
            if (result)
                log_info("Success - %s decoding finished", options->format);
            else
                log_info("Failure - %s decoding finished", options->format);
            converter_destroy(converter);
        }
        else
        {
            result = false;
        }
    }

    return result;
}

static void set_file_path(VirtualFile *file, const char *input_path)
{
    char *output_path = new char[strlen(input_path) + 2];
    assert(output_path != nullptr);
    strcpy(output_path, input_path);
    strcat(output_path, "~");
    virtual_file_set_name(file, output_path);
    delete []output_path;
}

static VirtualFile *read_and_decode(void *_context)
{
    ReadContext *context = (ReadContext*)_context;

    IO *io = io_create_from_file(context->path_info->input_path, "rb");
    if (io == nullptr)
        return nullptr;

    VirtualFile *file = virtual_file_create();
    assert(file != nullptr);
    io_write_string_from_io(file->io, io,  io_size(io));
    io_destroy(io);
    io_seek(file->io, 0);

    set_file_path(
        file,
        context->options->output_dir == nullptr
            ? context->path_info->input_path
            : context->path_info->target_path);

    if (!guess_converter_and_decode(
        context->options,
        context->arg_parser,
        context->conv_factory,
        file))
    {
        virtual_file_destroy(file);
        return nullptr;
    }

    return file;
}

static bool run(
    Options *options,
    ArgParser &arg_parser,
    ConverterFactory *conv_factory)
{
    size_t i;
    assert(options != nullptr);
    assert(conv_factory != nullptr);

    OutputFiles *output_files = output_files_create_hdd(options->output_dir);
    assert(output_files != nullptr);

    ReadContext context =
    {
        options,
        arg_parser,
        conv_factory,
        nullptr,
    };

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
    Options options;
    options.format = nullptr;
    options.output_dir = nullptr;
    options.input_paths = nullptr;

    ConverterFactory *conv_factory = converter_factory_create();
    assert(conv_factory != nullptr);
    ArgParser arg_parser;
    arg_parser.parse(argc, argv);

    add_output_folder_option(arg_parser, &options);
    add_format_option(arg_parser, &options);
    add_quiet_option(arg_parser);
    add_help_option(arg_parser);

    if (should_show_help(arg_parser))
    {
        Converter *converter = options.format != nullptr
            ? converter_factory_from_string(conv_factory, options.format)
            : nullptr;
        print_help(argv[0], arg_parser, &options, converter);
        if (converter != nullptr)
            converter_destroy(converter);
    }
    else
    {
        if (!add_input_paths_option(arg_parser, &options))
            exit_code = 1;
        else if (!run(&options, arg_parser, conv_factory))
            exit_code = 1;
    }

    if (options.input_paths != nullptr)
    {
        size_t i;
        for (i = 0; i < array_size(options.input_paths); i ++)
        {
            PathInfo *pi = (PathInfo*)array_get(options.input_paths, i);
            delete []pi->input_path;
            delete []pi->target_path;
            delete pi;
        }
        array_destroy(options.input_paths);
    }
    converter_factory_destroy(conv_factory);
    return exit_code;
}
