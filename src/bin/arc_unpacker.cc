#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "arg_parser.h"
#include "bin_helpers.h"
#include "factory/archive_factory.h"
#include "file_io.h"
#include "formats/archive.h"
#include "logger.h"
#include "output_files.h"
#include "virtual_file.h"

namespace
{
    typedef struct
    {
        std::string format;
        std::string input_path;
        std::string output_path;
    } Options;

    std::string get_default_output_path(const std::string input_path)
    {
        return input_path + "~";
    }

    void add_format_option(ArgParser &arg_parser, Options &options)
    {
        arg_parser.add_help(
            "-f, --fmt=FORMAT",
            "Selects the archive format.");

        if (arg_parser.has_switch("-f"))
            options.format = arg_parser.get_switch("-f");
        if (arg_parser.has_switch("--fmt"))
            options.format = arg_parser.get_switch("--fmt");
    }

    void add_path_options(ArgParser &arg_parser, Options &options)
    {
        const std::vector<std::string> stray = arg_parser.get_stray();
        if (stray.size() < 2)
        {
            log_error("Required more arguments.");
            exit(1);
        }

        options.input_path = stray[1];
        options.output_path = stray.size() < 3
            ? get_default_output_path(stray[1])
            : stray[2];
    }

    void print_help(
        const std::string path_to_self,
        ArgParser &arg_parser,
        Options &options,
        Archive *archive)
    {
        printf("Usage: %s [options] [arc_options] input_path [output_path]\n",
            path_to_self.c_str());

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
            printf("[arc_options] specific to %s:\n", options.format.c_str());
            puts("");
            arg_parser.print_help();
            return;
        }
        puts("[arc_options] depend on each archive and are required at "
            "runtime.");
        puts("See --help --fmt=FORMAT to get detailed help for given "
            "archive.");
    }

    void unpack(
        Archive &archive,
        ArgParser &arg_parser,
        IO &io,
        OutputFiles &output_files)
    {
        io.seek(0);
        archive.parse_cli_options(arg_parser);
        archive.unpack(io, output_files);
    }

    bool guess_archive_and_unpack(
        IO &io,
        OutputFiles &output_files,
        Options &options,
        ArgParser &arg_parser,
        const ArchiveFactory &arc_factory)
    {
        if (options.format == "")
        {
            for (auto& format : arc_factory.get_formats())
            {
                std::unique_ptr<Archive> archive(
                    arc_factory.create_archive(format));

                try
                {
                    log_info("Trying %s...", format.c_str());
                    unpack(*archive, arg_parser, io, output_files);
                    log_info(
                        "Success - %s unpacking finished successfully.",
                        format.c_str());
                    return true;
                }
                catch (std::exception &e)
                {
                    log_error("%s", e.what());
                    log_info(
                        "Failure - %s didn\'t work, trying next format...",
                        format.c_str());
                    if (log_enabled(LOG_LEVEL_INFO))
                        puts("");
                }
            }

            log_error("Nothing left to try. File not recognized.");
            return false;
        }
        else
        {
            std::unique_ptr<Archive> archive(
                arc_factory.create_archive(options.format));

            try
            {
                unpack(*archive, arg_parser, io, output_files);
                log_info(
                    "Success - %s unpacking finished", options.format.c_str());
                return true;
            }
            catch (std::exception &e)
            {
                log_error("%s", e.what());
                log_info(
                    "Failure - %s unpacking finished", options.format.c_str());
                return false;
            }
        }
    }

    bool run(
        Options &options,
        ArgParser &arg_parser,
        const ArchiveFactory &arc_factory)
    {
        OutputFilesHdd output_files(options.output_path);
        FileIO io(options.input_path, "rb");
        return guess_archive_and_unpack(
            io,
            output_files,
            options,
            arg_parser,
            arc_factory);
    }
}

int main(int argc, const char **argv)
{
    try
    {
        int exit_code = 0;
        Options options;
        ArchiveFactory arc_factory;
        ArgParser arg_parser;
        arg_parser.parse(argc, argv);

        add_format_option(arg_parser, options);
        add_quiet_option(arg_parser);
        add_help_option(arg_parser);

        if (should_show_help(arg_parser))
        {
            std::unique_ptr<Archive> archive(options.format != ""
                ? arc_factory.create_archive(options.format)
                : nullptr);
            print_help(argv[0], arg_parser, options, archive.get());
        }
        else
        {
            add_path_options(arg_parser, options);
            if (!run(options, arg_parser, arc_factory))
                exit_code = 1;
        }

        return exit_code;
    }
    catch (std::exception &e)
    {
        log_error("%s", e.what());
        return 1;
    }
}
