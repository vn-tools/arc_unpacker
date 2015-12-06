#include "fmt/shiina_rio/warc_archive_decoder.h"
#include <set>
#include "algo/str.h"
#include "err.h"
#include "fmt/shiina_rio/warc/decompress.h"
#include "fmt/shiina_rio/warc/decrypt.h"
#include "fmt/shiina_rio/warc/plugin_registry.h"
#include "io/memory_stream.h"
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

bool WarcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    WarcArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    const auto plugin = p->plugin_registry.get_plugin();
    if (!plugin)
        throw err::UsageError("Plugin not specified");

    input_file.stream.seek(magic.size());
    const int warc_version = 100
        * algo::from_string<float>(input_file.stream.read(3).str());
    if (warc_version != 170)
        throw err::UnsupportedVersionError(warc_version);

    input_file.stream.seek(8);
    const auto table_offset = input_file.stream.read_u32_le() ^ 0xF182AD82;
    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read_to_eof();

    warc::decrypt_table_data(*plugin, warc_version, table_offset, table_data);
    io::MemoryStream table_stream(table_data);

    std::set<io::path> known_names;
    auto meta = std::make_unique<ArchiveMetaImpl>(*plugin, warc_version);
    while (!table_stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name = table_stream.read_to_zero(plugin->entry_name_size).str();
        for (auto &c : name)
        {
            if (static_cast<const u8>(c) >= 0x80)
            {
                c = '_';
                entry->suspicious = true;
            }
        }
        entry->path = name;
        entry->suspicious |= known_names.find(entry->path) != known_names.end();
        known_names.insert(entry->path);
        entry->offset = table_stream.read_u32_le();
        entry->size_comp = table_stream.read_u32_le();
        entry->size_orig = table_stream.read_u32_le();
        entry->time = table_stream.read_u64_le();
        entry->flags = table_stream.read_u32_le();
        if (!entry->path.str().empty())
            meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> WarcArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);

    const bstr head = input_file.stream.read(4);
    const auto size_orig = input_file.stream.read_u32_le();
    const bool compress_crypt = head[3] > 0;
    const u32 tmp = *head.get<u32>() ^ 0x82AD82 ^ size_orig;
    bstr file_magic(3);
    file_magic[0] = tmp & 0xFF;
    file_magic[1] = (tmp >> 8) & 0xFF;
    file_magic[2] = (tmp >> 16) & 0xFF;

    auto data = input_file.stream.read(entry->size_comp - 8);
    if (entry->flags & 0x80000000)
        warc::decrypt_essential(meta->plugin, meta->warc_version, data);
    if (meta->plugin.flag_pre_crypt)
        meta->plugin.flag_pre_crypt(data, 0x202);
    if (entry->flags & 0x20000000)
    {
        if (!meta->plugin.crc_crypt)
            throw err::NotSupportedError("CRC crypt is not implemented");
        meta->plugin.crc_crypt(data);
    }

    std::function<bstr(const bstr &, const size_t, const bool)> decompressor;
    if (file_magic == "YH1"_b)
        decompressor = warc::decompress_yh1;
    else if (file_magic == "YLZ"_b)
        decompressor = warc::decompress_ylz;
    else if (file_magic == "YPK"_b)
        decompressor = warc::decompress_ypk;

    if (decompressor)
    {
        data = decompressor(data, size_orig, compress_crypt);
        if (entry->flags & 0x40000000)
        {
            if (!meta->plugin.crc_crypt)
                throw err::NotSupportedError("CRC crypt is not implemented");
            meta->plugin.crc_crypt(data);
        }
        if (meta->plugin.flag_post_crypt)
            meta->plugin.flag_post_crypt(data, 0x204);
    }

    if (entry->suspicious)
    {
        Log.warn(
            "Suspicious entry: %s (anti-extract decoy?)\n",
            entry->path.c_str());
    }

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> WarcArchiveDecoder::get_linked_formats() const
{
    return {"shiina-rio/ogv", "shiina-rio/s25"};
}

static auto dummy = fmt::register_fmt<WarcArchiveDecoder>("shiina-rio/warc");
