#include "fmt/alice_soft/ald_archive_decoder.h"
#include "fmt/alice_soft/pms_image_decoder.h"
#include "fmt/alice_soft/qnt_image_decoder.h"
#include "fmt/alice_soft/vsp_image_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static u32 read_24_le(io::IO &io)
{
    return (io.read_u8() << 8) | (io.read_u8() << 16) | (io.read_u8() << 24);
}

struct AldArchiveDecoder::Priv final
{
    PmsImageDecoder pms_image_decoder;
    VspImageDecoder vsp_image_decoder;
    QntImageDecoder qnt_image_decoder;
};

AldArchiveDecoder::AldArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->pms_image_decoder);
    add_decoder(&p->vsp_image_decoder);
    add_decoder(&p->qnt_image_decoder);
}

AldArchiveDecoder::~AldArchiveDecoder()
{
}

bool AldArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.has_extension("ald");
}

std::unique_ptr<fmt::ArchiveMeta>
    AldArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto file_count = read_24_le(arc_file.io) / 3;

    std::vector<size_t> offsets(file_count);
    for (auto i : util::range(file_count))
        offsets[i] = read_24_le(arc_file.io);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto &offset : offsets)
    {
        if (!offset)
            break;
        arc_file.io.seek(offset);
        auto header_size = arc_file.io.read_u32_le();
        if (arc_file.io.tell() + header_size < arc_file.io.size())
        {
            auto entry = std::make_unique<ArchiveEntryImpl>();
            entry->size = arc_file.io.read_u32_le();
            arc_file.io.skip(8);
            auto name = arc_file.io.read_to_zero(header_size - 16);
            entry->name = util::sjis_to_utf8(name).str();
            entry->offset = arc_file.io.tell();
            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

std::unique_ptr<File> AldArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<AldArchiveDecoder>("alice/ald");
