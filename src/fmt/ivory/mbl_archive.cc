// MBL archive
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: .mbl
//
// Known games:
// - Candy Toys
// - Wanko to Kurasou

#include <map>
#include "fmt/ivory/mbl_archive.h"
#include "fmt/ivory/prs_converter.h"
#include "fmt/ivory/wady_converter.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/range.h"
#include "log.h"

using namespace au;
using namespace au::fmt::ivory;

namespace
{
    enum Version
    {
        Unknown,
        Version1,
        Version2,
    };

    struct TableEntry
    {
        std::string name;
        size_t offset;
        size_t size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;

    using PluginFunc = std::function<void(bstr &)>;
    using PluginTable = std::map<std::string, PluginFunc>;
}

static int check_version(io::IO &arc_io, size_t file_count, size_t name_size)
{
    arc_io.skip((file_count - 1) * (name_size + 8));
    arc_io.skip(name_size);
    auto last_file_offset = arc_io.read_u32_le();
    auto last_file_size = arc_io.read_u32_le();
    return last_file_offset + last_file_size == arc_io.size();
}

static Version get_version(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    if (check_version(arc_io, file_count, 16))
        return Version::Version1;

    arc_io.seek(4);
    auto name_size = arc_io.read_u32_le();
    if (check_version(arc_io, file_count, name_size))
        return Version::Version2;

    return Version::Unknown;
}

static Table read_table(io::IO &arc_io, Version version)
{
    Table table;
    auto file_count = arc_io.read_u32_le();
    auto name_size = version == Version::Version2 ? arc_io.read_u32_le() : 16;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::sjis_to_utf8(arc_io.read_to_zero(name_size)).str();
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, bool encrypted, PluginFunc decrypt)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);
    if (encrypted)
    {
        if (!decrypt)
            throw std::runtime_error("File is decrypted, but plugin not set.");
        decrypt(data);
    }
    file->io.write(data);
    file->name = entry.name;
    return file;
}

struct MblArchive::Priv
{
    PrsConverter prs_converter;
    WadyConverter wady_converter;
    PluginFunc plugin;
    PluginTable all_plugins;
};

MblArchive::MblArchive() : p(new Priv)
{
    add_transformer(&p->prs_converter);
    add_transformer(&p->wady_converter);

    p->all_plugins["candy"] = [](bstr &data)
        {
            for (auto i : util::range(data.size()))
                data[i] = -data[i];
        };

    p->all_plugins["wanko"] = [](bstr &data)
        {
            static const bstr key =
                "\x82\xED\x82\xF1\x82\xB1"
                "\x88\xC3\x8D\x86\x89\xBB"_b;
            for (auto i : util::range(data.size()))
                data[i] ^= key[i % key.size()];
        };

    p->all_plugins["noop"] = [](bstr &)
        {
        };
}

MblArchive::~MblArchive()
{
}

void MblArchive::register_cli_options(ArgParser &arg_parser) const
{
    auto sw = arg_parser.register_switch({"-p", "--plugin"})
        ->set_value_name("PLUGIN")
        ->set_description("Specifies plugin for decoding dialog files.");
    for (auto &it : p->all_plugins)
        sw->add_possible_value(it.first);
    Archive::register_cli_options(arg_parser);
}

void MblArchive::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("--plugin"))
        set_plugin(arg_parser.get_switch("--plugin"));
    Archive::parse_cli_options(arg_parser);
}

void MblArchive::set_plugin(const std::string &plugin_name)
{
    if (p->all_plugins.find(plugin_name) == p->all_plugins.end())
        throw std::runtime_error("Unknown plugin: " + plugin_name);
    p->plugin = p->all_plugins[plugin_name];
}

bool MblArchive::is_recognized_internal(File &arc_file) const
{
    return get_version(arc_file.io) != Version::Unknown;
}

void MblArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto version = get_version(arc_file.io);
    bool encrypted = arc_file.name.find("mg_data") != std::string::npos;
    arc_file.io.seek(0);

    auto table = read_table(arc_file.io, version);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry, encrypted, p->plugin);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<MblArchive>("ivory/mbl");
