#include "fmt/nitroplus/npa_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "fmt/nitroplus/npa_filter_registry.h"
#include "util/plugin_mgr.h"

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
        std::shared_ptr<NpaFilter> filter;
        u32 key1, key2;
        bool files_are_encrypted;
        bool files_are_compressed;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        bstr path_orig;
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
    for (auto char_pos : algo::range(name.size()))
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
        name[char_pos] += (key & 0xFF);
    }
}

static void decrypt_file_data(
    const ArchiveMetaImpl &meta, const ArchiveEntryImpl &entry, bstr &data)
{
    u32 key = meta.filter->data_key;
    for (auto i : algo::range(entry.path_orig.size()))
        key -= entry.path_orig[i];
    key *= entry.path_orig.size();
    key += meta.key1 * meta.key2;
    key *= entry.size_orig;
    key &= 0xFF;

    u8 *data_ptr = data.get<u8>();
    size_t size = 0x1000 + entry.path_orig.size();
    for (auto i : algo::range(std::min(size, data.size())))
        data_ptr[i] = meta.filter->permutation[data_ptr[i]] - key - i;
}

struct NpaArchiveDecoder::Priv final
{
    NpaFilterRegistry filter_registry;
};

NpaArchiveDecoder::NpaArchiveDecoder() : p(new Priv)
{
}

NpaArchiveDecoder::~NpaArchiveDecoder()
{
}

void NpaArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    p->filter_registry.register_cli_options(arg_parser);
}

void NpaArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter_registry.parse_cli_options(arg_parser);
}

bool NpaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta> NpaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->filter = p->filter_registry.get_filter();
    if (!meta->filter)
        throw err::UsageError("Plugin not specified");

    input_file.stream.seek(magic.size());
    meta->key1 = input_file.stream.read_u32_le();
    meta->key2 = input_file.stream.read_u32_le();
    meta->files_are_compressed = input_file.stream.read_u8() > 0;
    meta->files_are_encrypted = input_file.stream.read_u8() > 0;
    const auto total_entry_count = input_file.stream.read_u32_le();
    const auto folder_count = input_file.stream.read_u32_le();
    const auto file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(8);
    const auto table_size = input_file.stream.read_u32_le();
    const auto data_offset = input_file.stream.tell() + table_size;

    for (const auto i : algo::range(total_entry_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        const auto name_size = input_file.stream.read_u32_le();
        entry->path_orig = input_file.stream.read(name_size);
        decrypt_file_name(*meta, entry->path_orig, i);
        entry->path = algo::sjis_to_utf8(entry->path_orig).str();

        FileType file_type = static_cast<FileType>(input_file.stream.read_u8());
        input_file.stream.skip(4);

        entry->offset = input_file.stream.read_u32_le() + data_offset;
        entry->size_comp = input_file.stream.read_u32_le();
        entry->size_orig = input_file.stream.read_u32_le();

        if (file_type == FILE_TYPE_DIRECTORY)
            continue;
        if (file_type != FILE_TYPE_FILE)
            throw err::NotSupportedError("Unknown file type");
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> NpaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);

    if (meta->files_are_encrypted)
        decrypt_file_data(*meta, *entry, data);

    if (meta->files_are_compressed)
        data = algo::pack::zlib_inflate(data);

    return std::make_unique<io::File>(entry->path, data);
}

static auto dummy = fmt::register_fmt<NpaArchiveDecoder>("nitroplus/npa");
