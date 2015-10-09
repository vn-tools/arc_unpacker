#include "fmt/cronus/pak_archive_decoder.h"
#include "err.h"
#include "fmt/cronus/common.h"
#include "fmt/cronus/grp_image_decoder.h"
#include "io/buffered_io.h"
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
    File &arc_file, const Plugin &plugin, bool encrypted)
{
    auto file_count = arc_file.io.read_u32_le() ^ plugin.key1;
    auto file_data_start = arc_file.io.read_u32_le() ^ plugin.key2;

    auto table_size_orig = file_count * 24;
    auto table_size_comp = file_data_start - arc_file.io.tell();
    auto table_data = arc_file.io.read(table_size_comp);
    if (encrypted)
    {
        auto key = get_delta_key("CHERRYSOFT"_b);
        delta_decrypt(table_data, key);
        table_data = util::pack::lzss_decompress_bytewise(
            table_data, table_size_orig);
    }
    io::BufferedIO table_io(table_data);

    auto meta = std::make_unique<fmt::ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read_to_zero(16).str();
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size = table_io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

struct PakArchiveDecoder::Priv final
{
    GrpImageDecoder grp_image_decoder;
    util::PluginManager<Plugin> plugin_mgr;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv)
{
    p->plugin_mgr.add("default", "Unencrypted games", {0, 0});
    p->plugin_mgr.add("sweet", "Sweet Pleasure", {0xBC138744, 0x64E0BA23});
    add_decoder(&p->grp_image_decoder);
}

PakArchiveDecoder::~PakArchiveDecoder()
{
}

bool PakArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (arc_file.io.read(magic2.size()) == magic2)
        return true;
    arc_file.io.seek(0);
    return arc_file.io.read(magic3.size()) == magic3;
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(File &arc_file) const
{
    if (arc_file.io.read(magic2.size()) != magic2)
        arc_file.io.seek(magic3.size());
    bool encrypted = arc_file.io.read_u32_le() > 0;
    auto pos = arc_file.io.tell();
    for (auto &plugin : p->plugin_mgr.get_all())
    {
        arc_file.io.seek(pos);
        try
        {
            auto meta = ::read_meta(arc_file, plugin, encrypted);
            if (meta->entries.size())
                return meta;
        }
        catch (...)
        {
            continue;
        }
    }
    throw err::RecognitionError("Unknown encryption scheme");
}

std::unique_ptr<File> PakArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("cronus/pak");
