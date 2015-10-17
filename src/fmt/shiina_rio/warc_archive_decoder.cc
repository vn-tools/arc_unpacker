#include "fmt/shiina_rio/warc_archive_decoder.h"
#include <boost/lexical_cast.hpp>
#include <set>
#include "err.h"
#include "fmt/shiina_rio/warc/decompress.h"
#include "fmt/shiina_rio/warc/decrypt.h"
#include "fmt/shiina_rio/warc/plugin_registry.h"
#include "io/buffered_io.h"
#include "log.h"

using namespace au;
using namespace au::fmt::shiina_rio;

static const bstr magic = "WARC\x20"_b;

namespace
{
    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        ArchiveMetaImpl(const warc::Plugin plugin, const int warc_version);
        const warc::Plugin plugin;
        const int warc_version;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_comp;
        size_t size_orig;
        u64 time;
        u32 flags;
        bool suspicious;
    };
}

ArchiveMetaImpl::ArchiveMetaImpl(
    const warc::Plugin plugin, const int warc_version)
    : plugin(plugin), warc_version(warc_version)
{
}

struct WarcArchiveDecoder::Priv final
{
    warc::PluginRegistry plugin_registry;
};

WarcArchiveDecoder::WarcArchiveDecoder() : p(new Priv)
{
}

WarcArchiveDecoder::~WarcArchiveDecoder()
{
}

void WarcArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    p->plugin_registry.register_cli_options(arg_parser);
    ArchiveDecoder::register_cli_options(arg_parser);
}

void WarcArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    p->plugin_registry.parse_cli_options(arg_parser);
    ArchiveDecoder::parse_cli_options(arg_parser);
}

bool WarcArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    WarcArchiveDecoder::read_meta_impl(File &arc_file) const
{
    const auto plugin = p->plugin_registry.get_plugin();
    if (!plugin)
        throw err::UsageError("Plugin not specified");

    arc_file.io.seek(magic.size());
    const int warc_version
        = 100 * boost::lexical_cast<float>(arc_file.io.read(3).str());
    if (warc_version != 170)
        throw err::UnsupportedVersionError(warc_version);

    arc_file.io.seek(8);
    const auto table_offset = arc_file.io.read_u32_le() ^ 0xF182AD82;
    arc_file.io.seek(table_offset);
    auto table_data = arc_file.io.read_to_eof();

    warc::decrypt_table_data(*plugin, warc_version, table_offset, table_data);
    io::BufferedIO table_io(table_data);

    std::set<std::string> known_names;
    auto meta = std::make_unique<ArchiveMetaImpl>(*plugin, warc_version);
    while (!table_io.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read_to_zero(plugin->entry_name_size).str();
        for (auto &c : entry->name)
        {
            if (static_cast<u8>(c) >= 0x80)
            {
                c = '_';
                entry->suspicious = true;
            }
        }
        entry->suspicious |= known_names.find(entry->name) != known_names.end();
        known_names.insert(entry->name);
        entry->offset = table_io.read_u32_le();
        entry->size_comp = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        entry->time = table_io.read_u64_le();
        entry->flags = table_io.read_u32_le();
        if (entry->name.size())
            meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<File> WarcArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);

    const bstr head = arc_file.io.read(4);
    const auto size_orig = arc_file.io.read_u32_le();
    const u32 tmp = *head.get<u32>() ^ 0x82AD82 ^ size_orig;
    bstr file_magic(3);
    file_magic[0] = tmp & 0xFF;
    file_magic[1] = (tmp >> 8) & 0xFF;
    file_magic[2] = (tmp >> 16) & 0xFF;

    auto data = arc_file.io.read(entry->size_comp - 8);
    if (entry->flags & 0x80000000)
        warc::decrypt_essential(meta->plugin, meta->warc_version, data);
    if (meta->plugin.flag_crypt.pre1)
        warc::decrypt_with_flags1(meta->plugin, data, 0x202);
    if (meta->plugin.flag_crypt.pre2)
        warc::decrypt_with_flags2(meta->plugin, data, 0x202);
    if (entry->flags & 0x20000000)
        warc::decrypt_with_crc(meta->plugin, data);

    std::function<bstr(const bstr &, const size_t)> decompressor;
    if (file_magic == "YH1"_b)
        decompressor = warc::decompress_yh1;
    else if (file_magic == "YLZ"_b)
        decompressor = warc::decompress_ylz;
    else if (file_magic == "YPK"_b)
    {
        decompressor = [&](const bstr &input, const size_t size_orig)
            {
                return warc::decompress_ypk(data, size_orig, head[3] > 0);
            };
    }

    if (decompressor)
    {
        data = decompressor(data, size_orig);
        if (entry->flags & 0x40000000)
            warc::decrypt_with_crc(meta->plugin, data);
        if (meta->plugin.flag_crypt.post)
            warc::decrypt_with_flags1(meta->plugin, data, 0x204);
    }

    if (entry->suspicious)
    {
        Log.warn(
            "Suspicious entry: %s (anti-extract decoy?)\n",
            entry->name.c_str());
    }

    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<WarcArchiveDecoder>("shiina-rio/warc");
