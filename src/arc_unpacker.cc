#include "arc_unpacker.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <map>
#include "fmt/idecoder.h"
#include "fmt/naming_strategies.h"
#include "fmt/registry.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

namespace
{
    struct PathInfo final
    {
        std::string input_path;
        std::string base_name;
    };

    struct Options final
    {
        std::string format;
        boost::filesystem::path output_dir;
        std::vector<std::unique_ptr<PathInfo>> input_paths;
        bool overwrite;
        bool recurse;
        bool should_show_help;
        bool should_list_fmt;
    };
}

struct ArcUnpacker::Priv final
{
public:
    bool run();
    Priv(const std::vector<std::string> &arguments, const std::string &version);

private:
    void register_cli_options();
    void print_fmt_list() const;
    void print_cli_help() const;
    void parse_cli_options();

    std::unique_ptr<fmt::IDecoder> guess_decoder(File &file) const;
    bool guess_decoder_and_unpack(File &file, const std::string &base_name);
    void unpack(
        fmt::IDecoder &decoder, File &file, const std::string &base_name) const;

    fmt::Registry &registry;
    const std::vector<std::string> arguments;
    std::string version;

    ArgParser arg_parser;
    std::string path_to_self;
    Options options;
};

ArcUnpacker::Priv::Priv(
    const std::vector<std::string> &arguments, const std::string &version)
    : registry(fmt::Registry::instance()),
        arguments(arguments),
        version(version)
{
}

void ArcUnpacker::Priv::print_fmt_list() const
{
    for (auto &name : registry.get_names())
        Log.info("%s\n", name.c_str());
}

void ArcUnpacker::Priv::print_cli_help() const
{
    Log.info("arc_unpacker v" + version + "\n");
    Log.info("Extracts images and sounds from various visual novels.\n\n");
    Log.info("Usage: " + path_to_self + " \\\n");
    Log.info("       [options] [fmt_options] input_path [input_path...]\n\n");
    Log.info(
R"(Depending on the format, files will be saved either in a subdirectory
(archives), or aside the input files (images, music etc.). If no output
directory is provided, files are going to be saved inside current working
directory.

[options] can be:

)");

    arg_parser.print_help();

    if (options.format != "")
    {
        ArgParser decoder_arg_parser;
        auto decoder = registry.create(options.format);
        if (decoder)
        {
            decoder->register_cli_options(decoder_arg_parser);
            Log.info("[fmt_options] specific to " + options.format + ":\n\n");
            decoder_arg_parser.print_help();
            return;
        }
    }

    Log.info(
R"([fmt_options] depend on chosen format and are required at runtime.
See --help --fmt=FORMAT to get detailed help for given decoder.

)");
}

void ArcUnpacker::Priv::register_cli_options()
{
    arg_parser.register_flag({"-h", "--help"})
        ->set_description("Shows this message.");

    arg_parser.register_flag({"-r", "--rename"})
        ->set_description(
            "Renames existing target files.\nBy default, they're overwritten.");

    arg_parser.register_flag({"-q", "--quiet"})
        ->set_description("Disables all output.");

    arg_parser.register_flag({"--no-color", "--no-colors"})
        ->set_description("Disables colors in output.");

    arg_parser.register_flag({"--no-recurse"})
        ->set_description("Disables automatic decoding of nested files.");

    arg_parser.register_switch({"-o", "--out"})
        ->set_value_name("DIR")
        ->set_description("Specifies where to put the output files.");

    auto sw = arg_parser.register_switch({"-f", "--fmt"})
        ->set_value_name("FORMAT")
        ->set_description("Disables guessing and selects given format.")
        ->hide_possible_values();
    for (auto &name : registry.get_names())
        sw->add_possible_value(name);

    arg_parser.register_flag({"--list-fmt"})
        ->set_description("Lists available FORMAT values.");
}

void ArcUnpacker::Priv::parse_cli_options()
{
    path_to_self = arg_parser.get_stray()[0];

    options.should_show_help
        = arg_parser.has_flag("-h") || arg_parser.has_flag("--help");

    options.should_list_fmt = arg_parser.has_flag("--list-fmt");

    options.overwrite
        = !arg_parser.has_flag("-r") && !arg_parser.has_flag("--rename");

    if (arg_parser.has_flag("--no-color") || arg_parser.has_flag("--no-colors"))
        Log.disable_colors();

    if (arg_parser.has_flag("-q") || arg_parser.has_flag("--quiet"))
    {
        Log.mute();
        Log.unmute(util::MessageType::Debug);
    }

    options.recurse = !arg_parser.has_flag("--no-recurse");

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

    const auto stray = arg_parser.get_stray();
    for (auto i : util::range(1, stray.size()))
    {
        auto path = stray[i];
        if (boost::filesystem::is_directory(path))
        {
            for (boost::filesystem::recursive_directory_iterator it(path);
                it != boost::filesystem::recursive_directory_iterator();
                it++)
            {
                std::unique_ptr<PathInfo> pi(new PathInfo);
                pi->input_path = it->path().string();
                pi->base_name = pi->input_path.substr(path.size());
                while (pi->base_name.size() > 0 &&
                    (pi->base_name[0] == '/' || pi->base_name[0] == '\\'))
                {
                    pi->base_name = pi->base_name.substr(1);
                }
                if (!boost::filesystem::is_directory(*it))
                    options.input_paths.push_back(std::move(pi));
            }
        }
        else
        {
            std::unique_ptr<PathInfo> pi(new PathInfo);
            pi->input_path = path;
            pi->base_name = boost::filesystem::path(path).filename().string();
            options.input_paths.push_back(std::move(pi));
        }
    }

    path_to_self = arg_parser.get_stray()[0];
}

bool ArcUnpacker::Priv::run()
{
    register_cli_options();
    arg_parser.parse(arguments);
    parse_cli_options();

    if (options.should_show_help)
    {
        print_cli_help();
        return true;
    }

    if (options.should_list_fmt)
    {
        print_fmt_list();
        return true;
    }

    if (options.input_paths.size() < 1)
    {
        Log.err("Error: required more arguments.\n\n");
        print_cli_help();
        return false;
    }

    bool result = true;
    for (auto &path_info : options.input_paths)
    {
        File file(path_info->input_path, io::FileMode::Read);

        auto tmp = boost::filesystem::path(path_info->base_name);
        auto base_name = tmp.stem().string() + "~" + tmp.extension().string();

        result &= guess_decoder_and_unpack(file, base_name);
    }
    return result;
}

void ArcUnpacker::Priv::unpack(
    fmt::IDecoder &decoder, File &file, const std::string &base_name) const
{
    FileSaverHdd file_saver(options.output_dir, options.overwrite);
    FileSaverCallback file_saver_proxy([&](std::shared_ptr<File> saved_file)
    {
        saved_file->name = decoder.naming_strategy()->decorate(
            base_name, saved_file->name);
        file_saver.save(saved_file);
    });

    ArgParser decoder_arg_parser;
    decoder.register_cli_options(decoder_arg_parser);
    decoder_arg_parser.parse(arguments);
    decoder.parse_cli_options(decoder_arg_parser);
    decoder.unpack(file, file_saver_proxy, options.recurse);
}

std::unique_ptr<fmt::IDecoder> ArcUnpacker::Priv::guess_decoder(
    File &file) const
{
    std::map<std::string, std::unique_ptr<fmt::IDecoder>> decoders;

    for (auto &name : registry.get_names())
    {
        auto current_decoder = registry.create(name);
        if (current_decoder->is_recognized(file))
            decoders[name] = std::move(current_decoder);
    }

    if (decoders.size() == 1)
    {
        Log.success(util::format(
            "File was recognized as %s.\n",
            decoders.begin()->first.c_str()));
        return std::move(decoders.begin()->second);
    }

    if (decoders.size() == 0)
    {
        Log.err("File was not recognized by any decoder.\n\n");
        return nullptr;
    }

    Log.warn("File wa recognized by multiple decoders:\n");
    for (const auto &it : decoders)
        Log.warn("- " + it.first + "\n");
    Log.warn("Please provide --fmt and proceed manually.\n\n");
    return nullptr;
}

bool ArcUnpacker::Priv::guess_decoder_and_unpack(
    File &file, const std::string &base_name)
{
    Log.info(util::format("Unpacking %s...\n", file.name.c_str()));

    auto decoder = options.format != ""
        ? registry.create(options.format)
        : guess_decoder(file);

    if (!decoder)
        return false;

    try
    {
        unpack(*decoder, file, base_name);
        Log.success("Unpacking finished successfully.\n\n");
        return true;
    }
    catch (std::exception &e)
    {
        Log.err("Error: " + std::string(e.what()) + "\n");
        Log.err("Unpacking finished with errors.\n\n");
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

bool ArcUnpacker::run()
{
    return p->run();
}
