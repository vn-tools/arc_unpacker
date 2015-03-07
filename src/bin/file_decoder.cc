#include <boost/filesystem.hpp>
#include <cstring>
#include "arg_parser.h"
#include "bin_helpers.h"
#include "compat/main.h"
#include "factory/converter_factory.h"
#include "file_io.h"
#include "file_saver.h"
#include "formats/converter.h"
#include "logger.h"

namespace
{
    typedef struct
    {
        std::string input_path;
        std::string target_path;
    } PathInfo;

    typedef struct
    {
        std::string format;
        std::string output_dir;
        std::vector<std::unique_ptr<PathInfo>> input_paths;
    } Options;

    void print_help(
        const std::string &path_to_self,
        ArgParser &arg_parser,
        const Options &options,
        const Converter *converter)
    {
        log(
            "Usage: %s [options] [file_options] input_path [input_path...]",
            path_to_self.c_str());

        log("\n");
        log("[options] can be:\n");
        log("\n");
        arg_parser.print_help();
        arg_parser.clear_help();
        log("\n");

        if (converter != nullptr)
        {
            converter->add_cli_help(arg_parser);
            log("[file_options] specific to %s:\n", options.format.c_str());
            log("\n");
            arg_parser.print_help();
            return;
        }
        log("[file_options] depend on chosen format and are required at "
            "runtime.\n");
        log("See --help --fmt=FORMAT to get detailed help for given "
            "converter.\n");
    }

    void add_output_folder_option(ArgParser &arg_parser, Options &options)
    {
        arg_parser.add_help(
            "-o, --out=FOLDER",
            "Where to put the output files.");

        if (arg_parser.has_switch("-o"))
            options.output_dir = arg_parser.get_switch("-o");
        if (arg_parser.has_switch("--out"))
            options.output_dir = arg_parser.get_switch("--out");
    }

    void add_format_option(ArgParser &arg_parser, Options &options)
    {
        arg_parser.add_help(
            "-f, --fmt=FORMAT",
            "Selects the file format.");

        if (arg_parser.has_switch("-f"))
            options.format = arg_parser.get_switch("-f");
        if (arg_parser.has_switch("--fmt"))
            options.format = arg_parser.get_switch("--fmt");
    }

    bool add_input_paths_option(ArgParser &arg_parser, Options &options)
    {
        const std::vector<std::string> stray = arg_parser.get_stray();
        for (size_t i = 1; i < stray.size(); i ++)
        {
            std::string path = stray[i];
            if (boost::filesystem::is_directory(path))
            {
                for (boost::filesystem::recursive_directory_iterator it(path);
                    it != boost::filesystem::recursive_directory_iterator();
                    it ++)
                {
                    std::unique_ptr<PathInfo> pi(new PathInfo);
                    pi->input_path = it->path().string();
                    pi->target_path = pi->input_path.substr(path.length() + 1);
                    options.input_paths.push_back(std::move(pi));
                }
            }
            else
            {
                std::unique_ptr<PathInfo> pi(new PathInfo);
                pi->input_path = path;
                pi->target_path
                    = boost::filesystem::path(path).filename().string();
                options.input_paths.push_back(std::move(pi));
            }
        }

        if (options.input_paths.size() < 1)
        {
            log("Error: required more arguments.\n");
            print_help(stray[0], arg_parser, options, nullptr);
            return false;
        }

        return true;
    }

    void decode(
        Converter &converter,
        ArgParser &arg_parser,
        File &file)
    {
        converter.parse_cli_options(arg_parser);
        converter.decode(file);
    }

    bool guess_converter_and_decode(
        const Options &options,
        ArgParser &arg_parser,
        const ConverterFactory &conv_factory,
        File &file)
    {
        if (options.format == "")
        {
            for (auto &format : conv_factory.get_formats())
            {
                std::unique_ptr<Converter> converter(
                    conv_factory.create_converter(format));

                try
                {
                    log("Trying %s... ", format.c_str());
                    decode(*converter, arg_parser, file);
                    return true;
                }
                catch (std::exception &e)
                {
                    log(
                        "Error: %s; trying next format...\n", e.what());
                }
            }

            log("Nothing left to try. File not recognized.\n");
            return false;
        }
        else
        {
            std::unique_ptr<Converter> converter(
                conv_factory.create_converter(options.format));

            try
            {
                decode(*converter, arg_parser, file);
                return true;
            }
            catch (std::exception &e)
            {
                log(
                    "Error: %s\nDecoding finished with errors.\n", e.what());
                return false;
            }
        }
    }

    std::unique_ptr<File> read_and_decode(
        Options &options,
        ArgParser &arg_parser,
        ConverterFactory &conv_factory,
        const PathInfo &path_info)
    {
        FileIO io(path_info.input_path, FileIOMode::Read);
        std::unique_ptr<File> file(new File);
        file->io.write_from_io(io, io.size());
        file->io.seek(0);

        file->name = options.output_dir == ""
            ? path_info.input_path
            : path_info.target_path;
        file->name += "~";

        if (!guess_converter_and_decode(
            options, arg_parser, conv_factory, *file))
        {
            throw std::runtime_error("Decoding failed");
        }

        return file;
    }

    bool run(
        Options &options,
        ArgParser &arg_parser,
        ConverterFactory &conv_factory)
    {
        FileSaverHdd file_saver(options.output_dir);

        bool result = true;
        for (auto &path_info : options.input_paths)
        {
            try
            {
                file_saver.save(std::move(read_and_decode(
                    options, arg_parser, conv_factory, *path_info)));
            }
            catch (std::runtime_error &)
            {
                result = false;
            }
        }
        return result;
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
            ConverterFactory conv_factory;
            ArgParser arg_parser;
            arg_parser.parse(args);

            add_output_folder_option(arg_parser, options);
            add_format_option(arg_parser, options);
            add_help_option(arg_parser);

            if (should_show_help(arg_parser))
            {
                std::unique_ptr<Converter> converter(options.format != ""
                    ? conv_factory.create_converter(options.format)
                    : nullptr);
                print_help(args[0], arg_parser, options, converter.get());
            }
            else
            {
                if (!add_input_paths_option(arg_parser, options))
                    exit_code = 1;
                else if (!run(options, arg_parser, conv_factory))
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
