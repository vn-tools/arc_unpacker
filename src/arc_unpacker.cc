#include "arc_unpacker.h"
#include <algorithm>
#include <map>
#include "arg_parser.h"
#include "fmt/decoder_util.h"
#include "fmt/idecoder.h"
#include "fmt/registry.h"
#include "io/filesystem.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

namespace
{
    struct Options final
    {
        std::string format;
        io::path output_dir;
        std::vector<io::path> input_paths;
        bool overwrite;
        bool enable_nested_decoding;
        bool should_show_help;
        bool should_list_fmt;
    };
}

struct ArcUnpacker::Priv final
{
public:
    Priv(const std::vector<std::string> &arguments, const std::string &version);
    int run() const;

private:
    void register_cli_options();
    void print_fmt_list() const;
    void print_cli_help() const;
    void parse_cli_options();

    bool guess_decoder_and_unpack(io::File &file) const;
    void unpack(fmt::IDecoder &decoder, io::File &file) const;

    const fmt::Registry &registry;
    const std::vector<std::string> arguments;
    const std::string version;

    ArgParser arg_parser;
    Options options;
};

static std::unique_ptr<fmt::IDecoder> guess_decoder(
    io::File &file, const fmt::Registry &registry)
{
    std::map<std::string, std::unique_ptr<fmt::IDecoder>> decoders;
    for (auto &name : registry.get_decoder_names())
    {
        auto current_decoder = registry.create_decoder(name);
        if (current_decoder->is_recognized(file))
            decoders[name] = std::move(current_decoder);
    }

    if (decoders.size() == 1)
    {
        Log.success(
            "File was recognized as %s.\n", decoders.begin()->first.c_str());
        return std::move(decoders.begin()->second);
    }

    if (decoders.empty())
    {
        Log.err("File was not recognized by any decoder.\n");
        return nullptr;
    }

    Log.warn("File wa recognized by multiple decoders:\n");
    for (const auto &it : decoders)
        Log.warn("- " + it.first + "\n");
    Log.warn("Please provide --fmt and proceed manually.\n");
    return nullptr;
}

ArcUnpacker::Priv::Priv(
    const std::vector<std::string> &arguments, const std::string &version)
    : registry(fmt::Registry::instance()),
        arguments(arguments),
        version(version)
{
    register_cli_options();
    arg_parser.parse(arguments);
    parse_cli_options();
}

void ArcUnpacker::Priv::print_fmt_list() const
{
    for (auto &name : registry.get_decoder_names())
        Log.info("%s\n", name.c_str());
}

void ArcUnpacker::Priv::print_cli_help() const
{
    Log.info(util::format(
R"(  __ _ _   _
 / _` | |_| |  arc_unpacker v%s
 \__,_|\__,_|  Extracts images and sounds from various visual novels.

Usage: arc_unpacker [options] [fmt_options] input_path [input_path...]

[options] can be:

)", version.c_str()));

    arg_parser.print_help();

    if (!options.format.empty())
    {
        auto decoder = registry.create_decoder(options.format);
        ArgParser decoder_arg_parser;
        decoder->register_cli_options(decoder_arg_parser);
        Log.info("[fmt_options] specific to " + options.format + ":\n\n");
        decoder_arg_parser.print_help();
    }
    else
    {
        Log.info(
R"([fmt_options] depend on chosen format and are required at runtime.
See --help --fmt=FORMAT to get detailed help for given decoder.

)");
    }

    Log.info(
R"(Useful places:
Source code   - https://github.com/vn-tools/arc_unpacker
Bug reporting - https://github.com/vn-tools/arc_unpacker/issues
Game requests - #arc_unpacker on Rizon
)");
}

void ArcUnpacker::Priv::register_cli_options()
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

    arg_parser.register_switch({"-o", "--out"})
        ->set_value_name("DIR")
        ->set_description("Specifies where to place the output files. "
            "By default, the files are placed in current working directory. "
            "(Archives always create an intermediate directory.)");

    auto sw = arg_parser.register_switch({"-f", "--fmt"})
        ->set_value_name("FORMAT")
        ->set_description("Disables guessing and selects given format.")
        ->hide_possible_values();
    for (auto &name : registry.get_decoder_names())
        sw->add_possible_value(name);

    arg_parser.register_flag({"-l", "--list-fmt"})
        ->set_description("Lists available FORMAT values.");
}

void ArcUnpacker::Priv::parse_cli_options()
{
    options.should_show_help
        = arg_parser.has_flag("-h") || arg_parser.has_flag("--help");

    options.should_list_fmt
        = arg_parser.has_flag("-l") || arg_parser.has_flag("--list-fmt");

    options.overwrite
        = !arg_parser.has_flag("-r") && !arg_parser.has_flag("--rename");

    if (arg_parser.has_flag("--no-color") || arg_parser.has_flag("--no-colors"))
        Log.disable_colors();

    if (arg_parser.has_flag("-q") || arg_parser.has_flag("--quiet"))
    {
        Log.mute();
        Log.unmute(util::MessageType::Debug);
    }

    options.enable_nested_decoding = !arg_parser.has_flag("--no-recurse");

    if (arg_parser.has_switch("-o"))
        options.output_dir = arg_parser.get_switch("-o");
    else if (arg_parser.has_switch("--out"))
        options.output_dir = arg_parser.get_switch("--out");
    else
        options.output_dir = "./";

    if (arg_parser.has_switch("-f"))
        options.format = arg_parser.get_switch("-f");
    if (arg_parser.has_switch("--fmt"))
        options.format = arg_parser.get_switch("--fmt");

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

int ArcUnpacker::Priv::run() const
{
    if (options.should_show_help)
    {
        print_cli_help();
        return 0;
    }

    if (options.should_list_fmt)
    {
        print_fmt_list();
        return 0;
    }

    if (options.input_paths.size() < 1)
    {
        Log.err("Error: required more arguments.\n\n");
        print_cli_help();
        return 1;
    }

    bool result = 0;
    size_t processed = 0;
    for (const auto &input_path : options.input_paths)
    {
        io::File file(io::absolute(input_path), io::FileMode::Read);
        result |= !guess_decoder_and_unpack(file);

        // keep one blank line between logs from each processed file
        processed++;
        const bool last = processed == options.input_paths.size();
        if (!last)
            Log.info("\n");
    }
    return result;
}

void ArcUnpacker::Priv::unpack(fmt::IDecoder &decoder, io::File &file) const
{
    const auto path = file.name;
    const auto base_name = path.stem() + "~" + path.extension();
    const FileSaverHdd saver(options.output_dir, options.overwrite);
    const FileSaverCallback saver_proxy(
        [&](std::shared_ptr<io::File> saved_file)
        {
            saved_file->name = fmt::decorate_path(
                decoder.naming_strategy(), base_name, saved_file->name);
            saver.save(saved_file);
        });

    return options.enable_nested_decoding
        ? fmt::unpack_recursive(arguments, decoder, file, saver_proxy, registry)
        : fmt::unpack_non_recursive(arguments, decoder, file, saver_proxy);
}

bool ArcUnpacker::Priv::guess_decoder_and_unpack(io::File &file) const
{
    Log.info(util::format("Unpacking %s...\n", file.name.str().c_str()));
    const auto decoder = options.format.empty()
        ? guess_decoder(file, registry)
        : registry.create_decoder(options.format);

    if (!decoder)
        return false;

    try
    {
        unpack(*decoder, file);
        Log.success("Unpacking finished successfully.\n");
        return true;
    }
    catch (std::exception &e)
    {
        Log.err("Error: " + std::string(e.what()) + "\n");
        Log.err("Unpacking finished with errors.\n");
        return false;
    }
}

ArcUnpacker::ArcUnpacker(
    const std::vector<std::string> &arguments, const std::string &version)
    : p(new Priv(arguments, version))
{
}

ArcUnpacker::~ArcUnpacker()
{
}

int ArcUnpacker::run() const
{
    return p->run();
}
