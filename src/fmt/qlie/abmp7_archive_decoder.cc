#include "fmt/qlie/abmp7_archive_decoder.h"
#include "util/encoding.h"

using namespace au;
using namespace au::fmt::qlie;

static const bstr magic = "ABMP7"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Abmp7ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Abmp7ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(12);
    arc_file.io.skip(arc_file.io.read_u32_le());

    auto meta = std::make_unique<ArchiveMeta>();

    auto first_entry = std::make_unique<ArchiveEntryImpl>();
    first_entry->name = "base.dat";
    first_entry->size = arc_file.io.read_u32_le();
    first_entry->offset = arc_file.io.tell();
    arc_file.io.skip(first_entry->size);
    meta->entries.push_back(std::move(first_entry));

    while (!arc_file.io.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto encoded_name = arc_file.io.read(arc_file.io.read_u8());
        arc_file.io.skip(31 - encoded_name.size());
        entry->name = util::sjis_to_utf8(encoded_name).str();
        if (entry->name.empty())
            entry->name = "unknown";
        entry->name += ".dat";
        entry->size = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.tell();
        arc_file.io.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Abmp7ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<Abmp7ArchiveDecoder>("qlie/abmp7");
