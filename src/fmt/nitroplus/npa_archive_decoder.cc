#include "fmt/nitroplus/npa_archive_decoder.h"
#include "fmt/nitroplus/npa_filters/chaos_head.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/plugin_mgr.hh"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

static const bstr magic = "NPA\x01\x00\x00\x00"_b;

namespace
{
    enum FileType
    {
        FILE_TYPE_DIRECTORY = 1,
        FILE_TYPE_FILE = 2
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        NpaFilter *filter;
        u32 key1, key2;
        bool files_are_encrypted;
        bool files_are_compressed;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        bstr name_orig;
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };

    struct TableEntry final
    {
        std::string name;
    };
}

static void decrypt_file_name(
    const ArchiveMetaImpl &meta, bstr &name, size_t file_index)
{
    u32 tmp = meta.filter->file_name_key(meta.key1, meta.key2);
    for (auto char_pos : util::range(name.size()))
    {
        u32 key = 0xFC * char_pos;
        key -= tmp >> 0x18;
        key -= tmp >> 0x10;
        key -= tmp >> 0x08;
        key -= tmp & 0xFF;
        key -= file_index >> 0x18;
        key -= file_index >> 0x10;
        key -= file_index >> 0x08;
        key -= file_index;
        name.get<u8>()[char_pos] += (key & 0xFF);
    }
}

static void decrypt_file_data(
    const ArchiveMetaImpl &meta, const ArchiveEntryImpl &entry, bstr &data)
{
    u32 key = meta.filter->data_key;
    for (auto i : util::range(entry.name_orig.size()))
        key -= entry.name_orig.get<u8>()[i];
    key *= entry.name_orig.size();
    key += meta.key1 * meta.key2;
    key *= entry.size_orig;
    key &= 0xFF;

    u8 *data_ptr = data.get<u8>();
    size_t size = 0x1000 + entry.name_orig.size();
    for (auto i : util::range(std::min(size, data.size())))
        data_ptr[i] = meta.filter->permutation[data_ptr[i]] - key - i;
}

struct NpaArchiveDecoder::Priv final
{
    util::PluginManager<std::function<std::unique_ptr<NpaFilter>()>> plugin_mgr;
    std::unique_ptr<NpaFilter> filter;
};

NpaArchiveDecoder::NpaArchiveDecoder() : p(new Priv)
{
    p->plugin_mgr.add("chaos_head", "ChaoS;HEAd", []()
        {
            auto filter = std::make_unique<NpaFilter>();
            npa_filters::chaos_head_filter_init(*filter);
            return filter;
        });
}

NpaArchiveDecoder::~NpaArchiveDecoder()
{
}

void NpaArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    p->plugin_mgr.register_cli_options(
        arg_parser, "Selects NPA decryption routine.");
    ArchiveDecoder::register_cli_options(arg_parser);
}

void NpaArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter = p->plugin_mgr.get_from_cli_options(arg_parser, true)();
    ArchiveDecoder::parse_cli_options(arg_parser);
}

bool NpaArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    NpaArchiveDecoder::read_meta_impl(File &arc_file) const
{
    if (!p->filter)
        throw err::UsageError("No plugin selected");

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->filter = p->filter.get();

    arc_file.io.seek(magic.size());
    meta->key1 = arc_file.io.read_u32_le();
    meta->key2 = arc_file.io.read_u32_le();
    meta->files_are_compressed = arc_file.io.read_u8() > 0;
    meta->files_are_encrypted = arc_file.io.read_u8() > 0;
    auto total_entry_count = arc_file.io.read_u32_le();
    auto folder_count = arc_file.io.read_u32_le();
    auto file_count = arc_file.io.read_u32_le();
    arc_file.io.skip(8);
    auto table_size = arc_file.io.read_u32_le();
    auto table_offset = arc_file.io.tell();

    for (auto i : util::range(total_entry_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        entry->name_orig = arc_file.io.read(arc_file.io.read_u32_le());
        decrypt_file_name(*meta, entry->name_orig, i);
        entry->name = util::sjis_to_utf8(entry->name_orig).str();

        FileType file_type = static_cast<FileType>(arc_file.io.read_u8());
        arc_file.io.skip(4);

        entry->offset = arc_file.io.read_u32_le() + table_offset + table_size;
        entry->size_comp = arc_file.io.read_u32_le();
        entry->size_orig = arc_file.io.read_u32_le();

        if (file_type == FILE_TYPE_DIRECTORY)
            continue;
        if (file_type != FILE_TYPE_FILE)
            throw err::NotSupportedError("Unknown file type");
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> NpaArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);

    if (meta->files_are_encrypted)
        decrypt_file_data(*meta, *entry, data);

    if (meta->files_are_compressed)
        data = util::pack::zlib_inflate(data);

    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<NpaArchiveDecoder>("nitro/npa");
