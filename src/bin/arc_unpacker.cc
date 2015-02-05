#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "arg_parser.h"
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

static std::string get_default_output_path(const std::string input_path)
{
    return input_path + "~";
}

static void add_format_option(ArgParser &arg_parser, Options *options)
{
    arg_parser.add_help(
        "-f, --fmt=FORMAT",
        "Selects the archive format.");

    if (arg_parser.has_switch("-f"))
        options->format = arg_parser.get_switch("-f").c_str();
    if (arg_parser.has_switch("--fmt"))
        options->format = arg_parser.get_switch("--fmt").c_str();
}

static void add_path_options(ArgParser &arg_parser, Options *options)
{
    std::vector<std::string> stray = arg_parser.get_stray();
    if (stray.size() < 2)
    {
        log_error("Required more arguments.");
        exit(1);
    }

    options->input_path = stray[1].c_str();
    options->output_path = stray.size() < 3
        ? get_default_output_path(stray[1]).c_str()
        : stray[2].c_str();
}

static void print_help(
    const char *path_to_self,
    ArgParser &arg_parser,
    Options *options,
    Archive *archive)
{
    assert(path_to_self != nullptr);
    assert(options != nullptr);

    printf("Usage: %s [options] [arc_options] input_path [output_path]\n",
        path_to_self);

    puts("");
    puts("Unless output path is provided, the script is going to use path");
    puts("followed with a tilde (~).");
    puts("");
    puts("[options] can be:");
    puts("");

    arg_parser.print_help();
    arg_parser.clear_help();
    puts("");

    if (archive != nullptr)
    {
        archive->add_cli_help(arg_parser);
        printf("[arc_options] specific to %s:\n", options->format);
        puts("");
        arg_parser.print_help();
        return;
    }
    puts("[arc_options] depend on each archive and are required at runtime.");
    puts("See --help --fmt=FORMAT to get detailed help for given archive.");
}

static bool unpack(
    Archive *archive, ArgParser &arg_parser, IO *io, OutputFiles *output_files)
{
    assert(archive != nullptr);
    io_seek(io, 0);
    archive->parse_cli_options(arg_parser);
    return archive->unpack(io, output_files);
}

static bool guess_archive_and_unpack(
    IO *io,
    OutputFiles *output_files,
    Options *options,
    ArgParser &arg_parser,
    ArchiveFactory *arc_factory)
{
    assert(io != nullptr);
    assert(options != nullptr);
    assert(arc_factory != nullptr);
    assert(output_files != nullptr);

    size_t i;
    bool result = false;
    if (options->format == nullptr)
    {
        const Array *format_strings = archive_factory_formats(arc_factory);
        assert(format_strings != nullptr);

        for (i = 0; i < array_size(format_strings); i ++)
        {
            const char *format = (const char*)array_get(format_strings, i);
            assert(format != nullptr);

            Archive *archive = archive_factory_from_string(arc_factory, format);
            assert(archive != nullptr);
            log_info("Trying %s...", format);
            result = unpack(archive, arg_parser, io, output_files);
            delete archive;

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
        delete archive;
    }
    return result;
}

static bool run(
    Options *options,
    ArgParser &arg_parser,
    ArchiveFactory *arc_factory)
{
    assert(options != nullptr);
    assert(arc_factory != nullptr);

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
    Options options;
    options.input_path = nullptr;
    options.output_path = nullptr;
    options.format = nullptr;

    ArchiveFactory *arc_factory = archive_factory_create();
    assert(arc_factory != nullptr);
    ArgParser arg_parser;
    arg_parser.parse(argc, argv);

    add_format_option(arg_parser, &options);
    add_quiet_option(arg_parser);
    add_help_option(arg_parser);

    if (should_show_help(arg_parser))
    {
        Archive *archive = options.format != nullptr
            ? archive_factory_from_string( arc_factory, options.format)
            : nullptr;
        print_help(argv[0], arg_parser, &options, archive);
        if (archive != nullptr)
            delete archive;
    }
    else
    {
        add_path_options(arg_parser, &options);
        if (!run(&options, arg_parser, arc_factory))
            exit_code = 1;
    }

    archive_factory_destroy(arc_factory);
    return exit_code;
}
