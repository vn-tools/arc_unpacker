#include "fmt/riddle_soft/pac_archive_decoder.h"
#include "err.h"
#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "PAC1"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct PacArchiveDecoder::Priv final
{
    CmpImageDecoder cmp_image_decoder;
};

PacArchiveDecoder::PacArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->cmp_image_decoder);
}

PacArchiveDecoder::~PacArchiveDecoder()
{
}

bool PacArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PacArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    auto file_data_start = arc_file.io.tell() + file_count * 32;
    auto current_file_offset = 0;
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(16).str();
        entry->size = arc_file.io.read_u32_le();
        auto prefix = arc_file.io.read(4);
        auto unk1 = arc_file.io.read_u32_le();
        auto unk2 = arc_file.io.read_u32_le();
        if (unk1 != unk2)
            throw err::CorruptDataError("Data mismatch");
        entry->offset = file_data_start + current_file_offset;
        current_file_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PacArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<PacArchiveDecoder>("riddle/pac");
