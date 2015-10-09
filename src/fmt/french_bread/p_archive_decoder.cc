#include "fmt/french_bread/p_archive_decoder.h"
#include "err.h"
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

bool PArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    auto meta = read_meta(arc_file);
    if (!meta->entries.size())
        return false;
    auto last_entry = static_cast<ArchiveEntryImpl*>(
        meta->entries[meta->entries.size() - 1].get());
    return last_entry->offset + last_entry->size == arc_file.io.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    PArchiveDecoder::read_meta_impl(File &arc_file) const
{
    static const u32 encryption_key = 0xE3DF59AC;
    arc_file.io.seek(0);
    auto magic = arc_file.io.read_u32_le();
    auto file_count = arc_file.io.read_u32_le() ^ encryption_key;
    if (magic != 0 && magic != 1)
        throw err::RecognitionError();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name = arc_file.io.read(60).str();
        for (auto j : util::range(name.size()))
            name[j] ^= i * j * 3 + 0x3D;
        entry->name = name.substr(0, name.find('\0'));
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le() ^ encryption_key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    static const size_t encrypted_block_size = 0x2173;
    for (auto i : util::range(std::min(encrypted_block_size, entry->size)))
        data[i] ^= entry->name[i % entry->name.size()] + i + 3;
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> PArchiveDecoder::get_linked_formats() const
{
    return { "fbread/ex3" };
}

static auto dummy = fmt::register_fmt<PArchiveDecoder>("fbread/p");
