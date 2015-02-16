#include <cstdlib>
#include <cstring>
#include "arg_parser.h"
#include "bin_helpers.h"
#include "compat/main.h"
#include "factory/archive_factory.h"
#include "file_io.h"
#include "formats/archive.h"
#include "logger.h"
#include "file_saver.h"
#include "file.h"

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

    void print_help(
        const std::string path_to_self,
        ArgParser &arg_parser,
        Options &options,
        Archive *archive)
    {
        log("Usage: %s [options] [arc_options] input_path [output_path]\n",
            path_to_self.c_str());

        log("\n");
        log("Unless output path is provided, the program is going to use ");
        log("input path\nfollowed with a tilde (~).\n");
        log("\n");
        log("[options] can be:\n");
        log("\n");

        arg_parser.print_help();
        arg_parser.clear_help();
        log("\n");

        if (archive != nullptr)
        {
            archive->add_cli_help(arg_parser);
            log("[arc_options] specific to %s:\n", options.format.c_str());
            log("\n");
            arg_parser.print_help();
            return;
        }
        log("[arc_options] depend on each archive and are required at "
            "runtime.\n");
        log("See --help --fmt=FORMAT to get detailed help for given "
            "archive.\n");
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
            log("Error: required more arguments.\n");
            print_help(stray[0], arg_parser, options, nullptr);
            exit(1);
        }

        options.input_path = stray[1];
        options.output_path = stray.size() < 3
            ? get_default_output_path(stray[1])
            : stray[2];
    }

    void unpack(
        Archive &archive,
        ArgParser &arg_parser,
        File &file,
        FileSaver &file_saver)
    {
        file.io.seek(0);
        archive.parse_cli_options(arg_parser);
        archive.unpack(file, file_saver);
    }

    bool guess_archive_and_unpack(
        File &file,
        FileSaver &file_saver,
        const Options &options,
        ArgParser &arg_parser,
        const ArchiveFactory &arc_factory)
    {
        if (options.format == "")
        {
            for (auto &format : arc_factory.get_formats())
            {
                std::unique_ptr<Archive> archive(
                    arc_factory.create_archive(format));

                try
                {
                    log("Trying %s...\n", format.c_str());
                    unpack(*archive, arg_parser, file, file_saver);
                    log("Unpacking finished successfully.\n");
                    return true;
                }
                catch (std::exception &e)
                {
                    log(
                        "Error: %s\n"
                        "Unpacking finished with errors, "
                        "trying next format...\n",
                        e.what());
                }
            }

            log("Nothing left to try. File not recognized.\n");
            return false;
        }
        else
        {
            std::unique_ptr<Archive> archive(
                arc_factory.create_archive(options.format));

            try
            {
                unpack(*archive, arg_parser, file, file_saver);
                log("Unpacking finished successfully.\n");
                return true;
            }
            catch (std::exception &e)
            {
                log(
                    "Error: %s\nUnpacking finished with errors.\n",
                    e.what());
                return false;
            }
        }
    }

    bool run(
        Options &options,
        ArgParser &arg_parser,
        const ArchiveFactory &arc_factory)
    {
        FileSaverHdd file_saver(options.output_path);
        File file(options.input_path, "rb");
        return guess_archive_and_unpack(
            file,
            file_saver,
            options,
            arg_parser,
            arc_factory);
    }
}

int main(int argc, const char **argv)
{
    return run_with_args(argc, argv, [](std::vector<std::string> args) -> int
    {
        try
        {
            int exit_code = 0;
            Options options;
            ArchiveFactory arc_factory;
            ArgParser arg_parser;
            arg_parser.parse(args);

            add_format_option(arg_parser, options);
            add_help_option(arg_parser);

            if (should_show_help(arg_parser))
            {
                std::unique_ptr<Archive> archive(options.format != ""
                    ? arc_factory.create_archive(options.format)
                    : nullptr);
                print_help(args[0], arg_parser, options, archive.get());
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
            log("Error: %s\n", e.what());
            return 1;
        }
    });
}
