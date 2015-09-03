#include <algorithm>
#include <boost/filesystem.hpp>
#include <map>
#include "arc_unpacker.h"
#include "fmt/registry.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

namespace
{
    struct PathInfo
    {
        std::string input_path;
        std::string base_name;
    };

    struct Options
    {
        std::string format;
        boost::filesystem::path output_dir;
        std::vector<std::unique_ptr<PathInfo>> input_paths;
        bool overwrite;
        bool recurse;
        bool should_show_help;
    };
}

static void register_cli_options(ArgParser &arg_parser)
{
    arg_parser.register_flag({"-h", "--help"}, "Shows this message.");
    arg_parser.register_flag(
        {"-r", "--rename"},
        "Renames existing target files.\nBy default, they're overwritten.");
    arg_parser.register_flag(
        {"--no-color", "--no-colors"}, "Disables color output.");
    arg_parser.register_flag(
        {"--no-recurse"}, "Disables automatic decoding of nested files.");
    arg_parser.register_switch(
        {"-o", "--out"}, "DIR", "Where to put the output files.");
    arg_parser.register_switch(
        {"-f", "--fmt"},
        "FORMAT",
        "Disables guessing and selects given format.");
}

static void parse_cli_options(ArgParser &arg_parser, Options &options)
{
    options.should_show_help
        = arg_parser.has_flag("-h") || arg_parser.has_flag("--help");

    options.overwrite
        = !arg_parser.has_flag("-r") && !arg_parser.has_flag("--rename");

    if (arg_parser.has_flag("--no-color") || arg_parser.has_flag("--no-colors"))
        Log.disable_colors();

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
}

struct ArcUnpacker::Priv
{
    Options options;
    ArgParser &arg_parser;
    fmt::Registry &registry;
    std::string version;

    Priv(ArgParser &arg_parser, const std::string &version)
        : arg_parser(arg_parser),
            registry(fmt::Registry::instance()),
            version(version)
    {
    }
};

ArcUnpacker::ArcUnpacker(ArgParser &arg_parser, const std::string &version)
    : p(new Priv(arg_parser, version))
{
    register_cli_options(arg_parser);
}

ArcUnpacker::~ArcUnpacker()
{
}

void ArcUnpacker::print_help(const std::string &path_to_self)
{
    Log.info("arc_unpacker v" + p->version + "\n");
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

    p->arg_parser.print_help();
    p->arg_parser.clear_help();
    Log.info("\n");

    if (p->options.format != "")
    {
        auto transformer = p->registry.create(p->options.format);
        if (transformer)
        {
            transformer->register_cli_options(p->arg_parser);
            Log.info(
                "[fmt_options] specific to " + p->options.format + ":\n\n");
            p->arg_parser.print_help();
            return;
        }
    }

    Log.info(
R"([fmt_options] depend on chosen format and are required at runtime.
See --help --fmt=FORMAT to get detailed help for given transformer.

Supported FORMAT values:

)");

    auto names = p->registry.get_names();
    size_t max_name_size = 0;
    for (auto &name : p->registry.get_names())
        max_name_size = std::max(name.size(), max_name_size);
    int columns = (79 - max_name_size - 2) / max_name_size;
    int rows = (names.size() + columns - 1) / columns;
    for (auto y : util::range(rows))
    {
        for (auto x : util::range(columns))
        {
            size_t i = x * rows + y;
            if (i >= names.size())
                continue;
            Log.info(util::format(
                util::format("- %%-%ds ", max_name_size),
                names[i].c_str()));
        }
        Log.info("\n");
    }
    Log.info("\n");
}

void ArcUnpacker::unpack(
    fmt::Transformer &transformer,
    File &file,
    const std::string &base_name) const
{
    FileSaverHdd file_saver(p->options.output_dir, p->options.overwrite);
    unpack(transformer, file, base_name, file_saver);
}

void ArcUnpacker::unpack(
    fmt::Transformer &transformer,
    File &file,
    const std::string &base_name,
    FileSaver &file_saver) const
{
    FileSaverCallback file_saver_proxy([&](std::shared_ptr<File> saved_file)
    {
        saved_file->name =
            fmt::FileNameDecorator::decorate(
                transformer.get_file_naming_strategy(),
                base_name,
                saved_file->name);
        file_saver.save(saved_file);
    });

    transformer.parse_cli_options(p->arg_parser);
    transformer.unpack(file, file_saver_proxy, p->options.recurse);
}

std::unique_ptr<fmt::Transformer> ArcUnpacker::guess_transformer(
    File &file) const
{
    std::map<std::string, std::unique_ptr<fmt::Transformer>> transformers;

    for (auto &name : p->registry.get_names())
    {
        auto current_transformer = p->registry.create(name);
        if (current_transformer->is_recognized(file))
            transformers[name] = std::move(current_transformer);
    }

    if (transformers.size() == 1)
    {
        Log.success(util::format(
            "File was recognized as %s.\n",
            transformers.begin()->first.c_str()));
        return std::move(transformers.begin()->second);
    }

    if (transformers.size() == 0)
    {
        Log.err("File was not recognized by any transformer.\n\n");
        return nullptr;
    }

    Log.warn("File wa recognized by multiple transformers:\n");
    for (const auto &it : transformers)
        Log.warn("- " + it.first + "\n");
    Log.warn("Please provide --fmt and proceed manually.\n\n");
    return nullptr;
}

bool ArcUnpacker::guess_transformer_and_unpack(
    File &file, const std::string &base_name) const
{
    Log.info(util::format("Unpacking %s...\n", file.name.c_str()));

    auto transformer = p->options.format != ""
        ? p->registry.create(p->options.format)
        : guess_transformer(file);

    if (!transformer)
        return false;

    try
    {
        unpack(*transformer, file, base_name);
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

bool ArcUnpacker::run()
{
    parse_cli_options(p->arg_parser, p->options);

    auto path_to_self = p->arg_parser.get_stray()[0];

    if (p->options.should_show_help)
    {
        print_help(path_to_self);
        return true;
    }

    if (p->options.input_paths.size() < 1)
    {
        Log.err("Error: required more arguments.\n\n");
        print_help(path_to_self);
        return false;
    }

    bool result = true;
    for (auto &path_info : p->options.input_paths)
    {
        File file(path_info->input_path, io::FileMode::Read);

        auto tmp = boost::filesystem::path(path_info->base_name);
        auto base_name = tmp.stem().string() + "~" + tmp.extension().string();

        result &= guess_transformer_and_unpack(file, base_name);
    }
    return result;
}
