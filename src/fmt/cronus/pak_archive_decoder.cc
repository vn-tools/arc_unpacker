#include "fmt/cronus/pak_archive_decoder.h"
#include "err.h"
#include "fmt/cronus/common.h"
#include "io/memory_stream.h"
#include "util/pack/lzss.h"
#include "util/plugin_mgr.hh"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cronus;

static const bstr magic2 = "CHERRY PACK 2.0\x00"_b;
static const bstr magic3 = "CHERRY PACK 3.0\x00"_b;

namespace
{
    struct Plugin final
    {
        u32 key1;
        u32 key2;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static std::unique_ptr<fmt::ArchiveMeta> read_meta(
    io::File &input_file, const Plugin &plugin, bool encrypted)
{
    auto file_count = input_file.stream.read_u32_le() ^ plugin.key1;
    auto file_data_start = input_file.stream.read_u32_le() ^ plugin.key2;
    if (file_data_start > input_file.stream.size())
        return nullptr;

    auto table_size_orig = file_count * 24;
    auto table_size_comp = file_data_start - input_file.stream.tell();
    auto table_data = input_file.stream.read(table_size_comp);
    if (encrypted)
    {
        auto key = get_delta_key("CHERRYSOFT"_b);
        delta_decrypt(table_data, key);
        table_data = util::pack::lzss_decompress_bytewise(
            table_data, table_size_orig);
    }
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = table_stream.read_to_zero(16).str();
        entry->offset = table_stream.read_u32_le() + file_data_start;
        entry->size = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

struct PakArchiveDecoder::Priv final
{
    util::PluginManager<Plugin> plugin_mgr;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv)
{
    p->plugin_mgr.add("default", "Unencrypted games", {0, 0});
    p->plugin_mgr.add("sweet", "Sweet Pleasure", {0xBC138744, 0x64E0BA23});
}

PakArchiveDecoder::~PakArchiveDecoder()
{
}

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic2.size()) == magic2)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic3.size()) == magic3;
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic2.size()) != magic2)
        input_file.stream.seek(magic3.size());
    bool encrypted = input_file.stream.read_u32_le() > 0;
    auto pos = input_file.stream.tell();
    for (auto &plugin : p->plugin_mgr.get_all())
    {
        input_file.stream.seek(pos);
        try
        {
            auto meta = ::read_meta(input_file, plugin, encrypted);
            if (meta && meta->entries.size())
                return meta;
        }
        catch (...)
        {
            continue;
        }
    }
    throw err::RecognitionError("Unknown encryption scheme");
}

std::unique_ptr<io::File> PakArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"cronus/grp"};
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("cronus/pak");
