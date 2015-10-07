#include "fmt/majiro/arc_archive_decoder.h"
#include "fmt/majiro/rc8_image_decoder.h"
#include "fmt/majiro/rct_image_decoder.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = "MajiroArcV3.000\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        u64 hash;
    };
}

struct ArcArchiveDecoder::Priv final
{
    Rc8ImageDecoder rc8_image_decoder;
    RctImageDecoder rct_image_decoder;
};

ArcArchiveDecoder::ArcArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->rc8_image_decoder);
    add_decoder(&p->rct_image_decoder);
}

ArcArchiveDecoder::~ArcArchiveDecoder()
{
}

bool ArcArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    ArcArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u32_le();
	auto names_offset = arc_file.io.read_u32_le();
    auto data_offset = arc_file.io.read_u32_le();

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->hash = arc_file.io.read_u64_le();
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }

    arc_file.io.seek(names_offset);
    for (auto &entry : meta->entries)
    {
        static_cast<ArchiveEntryImpl*>(entry.get())->name
            = util::sjis_to_utf8(arc_file.io.read_to_zero()).str();
    }

    return meta;
}

std::unique_ptr<File> ArcArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<ArcArchiveDecoder>("majiro/arc");
