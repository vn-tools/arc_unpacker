#include "fmt/sysadv/pak_archive_decoder.h"
#include "fmt/sysadv/pga_image_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::sysadv;

static const bstr magic = "\x05PACK2"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct PakArchiveDecoder::Priv final
{
    PgaImageDecoder pga_image_decoder;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->pga_image_decoder);
}

PakArchiveDecoder::~PakArchiveDecoder()
{
}

bool PakArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name = arc_file.io.read(arc_file.io.read_u8());
        for (auto i : util::range(name.size()))
            name[i] ^= 0xFF;
        entry->name = name.str();
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
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

static auto dummy = fmt::Registry::add<PakArchiveDecoder>("sysadv/pak");
