#include "flow/cli_facade.h"
#include <algorithm>
#include <map>
#include "algo/range.h"
#include "algo/str.h"
#include "arg_parser.h"
#include "dec/idecoder.h"
#include "dec/registry.h"
#include "flow/file_saver_hdd.h"
#include "flow/parallel_unpacker.h"
#include "io/file_system.h"
#include "version.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::flow;

namespace
{
    struct Options final
    {
        std::string decoder;
        io::path output_dir;
        std::vector<io::path> input_paths;
        bool overwrite;
        bool enable_nested_decoding;
        bool enable_virtual_file_system;
        bool should_show_help;
        bool should_show_version;
        bool should_list_decoders;
        unsigned int thread_count;
    };
}

struct CliFacade::Priv final
{
public:
    Priv(Logger &logger, const std::vector<std::string> &arguments);
    int run() const;

private:
    void register_cli_options();
    void print_decoder_list() const;
    void print_cli_help() const;
    void parse_cli_options();

    Logger &logger;
    const std::vector<std::string> arguments;
    const dec::Registry &registry;

    ArgParser arg_parser;
    Options options;
};

CliFacade::Priv::Priv(Logger &logger, const std::vector<std::string> &arguments)
    : logger(logger), arguments(arguments), registry(dec::Registry::instance())
{
    register_cli_options();
    arg_parser.parse(arguments);
    parse_cli_options();
}

void CliFacade::Priv::print_decoder_list() const
{
    for (auto &name : registry.get_decoder_names())
        logger.info("%s\n", name.c_str());
}

void CliFacade::Priv::print_cli_help() const
{
    logger.info(
R"(  __ _ _   _
 / _` | |_| |  arc_unpacker v%s
 \__,_|\__,_|  Extracts images and sounds from various visual novels.

Usage: arc_unpacker [options] [dec_options] input_path [input_path...]

[options] can be:

)", au::version_long.c_str());

    arg_parser.print_help(logger);

    if (!options.decoder.empty())
    {
        auto decoder = registry.create_decoder(options.decoder);
        ArgParser decoder_arg_parser;
        decoder->register_cli_options(decoder_arg_parser);
        logger.info("[dec_options] specific to " + options.decoder + ":\n\n");
        decoder_arg_parser.print_help(logger);
    }
    else
    {
        logger.info(
R"([dec_options] depend on the chosen decoder and are required at runtime.
See --help --dec=DECODER to get detailed help for given decoder.

)");
    }

    logger.info(
R"(Useful places:
Source code   - https://github.com/vn-tools/arc_unpacker
Bug reporting - https://github.com/vn-tools/arc_unpacker/issues
Game requests - #arc_unpacker on Rizon
)");
}

void CliFacade::Priv::register_cli_options()
{
    arg_parser.register_flag({"-h", "--help"})
        ->set_description("Shows this message.");

    arg_parser.register_flag({"-r", "--rename"})
        ->set_description(
            "Renames output files to preserve existing files. "
            "By default, existing files are overwritten with output files.");

    arg_parser.register_flag({"-q", "--quiet"})
        ->set_description("Disables all console output.");

    arg_parser.register_flag({"--no-color", "--no-colors"})
        ->set_description("Disables colors in console output.");

    arg_parser.register_flag({"--no-recurse"})
        ->set_description("Disables automatic decoding of nested files.");

    arg_parser.register_flag({"--no-vfs"})
        ->set_description("Disables virtual file system lookups.");

    arg_parser.register_switch({"-o", "--out"})
        ->set_value_name("DIR")
        ->set_description("Specifies where to place the output files. "
            "By default, the files are placed in current working directory. "
            "(Archives always create an intermediate directory.)");

    auto sw = arg_parser.register_switch({"-d", "--dec"})
        ->set_value_name("DECODER")
        ->set_description("Disables guessing and selects given decoder.")
        ->hide_possible_values();
    for (auto &name : registry.get_decoder_names())
        sw->add_possible_value(name);

    arg_parser.register_switch({"-t", "--threads"})
        ->set_value_name("NUM")
        ->set_description("Sets worker thread count.");

    arg_parser.register_flag({"-l", "--list-decoders"})
        ->set_description("Lists available DECODER values.");

    arg_parser.register_flag({"-v", "--version"})
        ->set_description("Shows arc_unpacker version.");
}

void CliFacade::Priv::parse_cli_options()
{
    options.should_show_help
        = arg_parser.has_flag("-h") || arg_parser.has_flag("--help");

    options.should_show_version
        = arg_parser.has_flag("-v") || arg_parser.has_flag("--version");

    options.should_list_decoders
        = arg_parser.has_flag("-l") || arg_parser.has_flag("--list-decoders");

    options.overwrite
        = !arg_parser.has_flag("-r") && !arg_parser.has_flag("--rename");

    if (arg_parser.has_flag("--no-color") || arg_parser.has_flag("--no-colors"))
        logger.disable_colors();

    if (arg_parser.has_flag("-q") || arg_parser.has_flag("--quiet"))
    {
        logger.mute();
        logger.unmute(Logger::MessageType::Debug);
    }

    options.enable_nested_decoding = !arg_parser.has_flag("--no-recurse");

    if (arg_parser.has_switch("-t"))
        options.thread_count = algo::from_string<int>(
            arg_parser.get_switch("-t"));
    else if (arg_parser.has_switch("--threads"))
        options.thread_count = algo::from_string<int>(
            arg_parser.get_switch("--threads"));
    else
        options.thread_count = 0;

    if (arg_parser.has_flag("--no-vfs"))
        VirtualFileSystem::disable();

    if (arg_parser.has_switch("-o"))
        options.output_dir = arg_parser.get_switch("-o");
    else if (arg_parser.has_switch("--out"))
        options.output_dir = arg_parser.get_switch("--out");
    else
        options.output_dir = "./";

    if (arg_parser.has_switch("-d"))
        options.decoder = arg_parser.get_switch("-d");
    if (arg_parser.has_switch("--dec"))
        options.decoder = arg_parser.get_switch("--dec");

    for (const auto &stray : arg_parser.get_stray())
    {
        if (io::is_directory(stray))
        {
            for (const auto &path : io::recursive_directory_range(stray))
                if (!io::is_directory(path))
                    options.input_paths.push_back(path);
        }
        else
        {
            options.input_paths.push_back(stray);
        }
    }
}

int CliFacade::Priv::run() const
{
    if (options.should_show_help)
    {
        print_cli_help();
        return 0;
    }

    if (options.should_show_version)
    {
        logger.info("%s\n", au::version_long.c_str());
        return 0;
    }

    if (options.should_list_decoders)
    {
        print_decoder_list();
        return 0;
    }

    if (options.input_paths.size() < 1)
    {
        logger.err("Error: required more arguments.\n\n");
        print_cli_help();
        return 1;
    }

    const auto available_decoders = options.decoder.empty()
        ? registry.get_decoder_names()
        : std::vector<std::string>{options.decoder};

    FileSaverHdd file_saver(options.output_dir, options.overwrite);
    ParallelUnpackerContext context(
        logger,
        file_saver,
        registry,
        options.enable_nested_decoding,
        arguments,
        available_decoders);

    ParallelUnpacker unpacker(context);
    for (const auto &input_path : options.input_paths)
    {
        unpacker.add_input_file(
            io::path(input_path).change_stem(input_path.stem() + "~").name(),
            [&]()
            {
                VirtualFileSystem::register_directory(
                    io::absolute(input_path).parent());
                return std::make_shared<io::File>(
                    io::absolute(input_path), io::FileMode::Read);
            });
    }
    return unpacker.run(options.thread_count) ? 0 : 1;
}

CliFacade::CliFacade(Logger &logger, const std::vector<std::string> &arguments)
    : p(new Priv(logger, arguments))
{
}

CliFacade::~CliFacade()
{
}

int CliFacade::run() const
{
    return p->run();
}
