#include "fmt/french_bread/p_archive_decoder.h"
#include "fmt/french_bread/ex3_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::french_bread;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static const u32 encryption_key = 0xE3DF59AC;

static std::string read_file_name(io::IO &arc_io, size_t file_id)
{
    std::string file_name = arc_io.read(60).str();
    for (auto i : util::range(60))
        file_name[i] ^= file_id * i * 3 + 0x3D;
    return file_name.substr(0, file_name.find('\0'));
}

struct PArchiveDecoder::Priv final
{
    Ex3ImageDecoder ex3_image_decoder;
};

PArchiveDecoder::PArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->ex3_image_decoder);
}

PArchiveDecoder::~PArchiveDecoder()
{
}

bool PArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    u32 magic = arc_file.io.read_u32_le();
    size_t file_count = arc_file.io.read_u32_le() ^ encryption_key;
    if (magic != 0 && magic != 1)
        return false;
    if (file_count > arc_file.io.size() || file_count * 68 > arc_file.io.size())
        return false;
    for (auto i : util::range(file_count))
    {
        read_file_name(arc_file.io, i);
        size_t offset = arc_file.io.read_u32_le();
        size_t size = arc_file.io.read_u32_le() ^ encryption_key;
        if (offset + size > arc_file.io.size())
            return false;
    }
    return true;
}

std::unique_ptr<fmt::ArchiveMeta>
    PArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(4);
    auto meta = std::make_unique<ArchiveMeta>();
    auto file_count = arc_file.io.read_u32_le() ^ encryption_key;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<ArchiveEntryImpl> entry(new ArchiveEntryImpl);
        entry->name = read_file_name(arc_file.io, i);
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le() ^ encryption_key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);

    static const size_t block_size = 0x2172;
    for (auto i : util::range(std::min(block_size + 1, entry->size)))
        data[i] ^= entry->name[i % entry->name.size()] + i + 3;

    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<PArchiveDecoder>("fbread/p");
