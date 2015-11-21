#include "fmt/cri/afs2_archive_decoder.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cri;

static const bstr magic = "AFS2"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Afs2ArchiveDecoder::is_recognized_impl(File &input_file) const
{
    input_file.stream.seek(0);
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Afs2ArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto file_count = input_file.stream.read_u32_le() - 1;
    input_file.stream.skip(4);
    input_file.stream.skip((file_count + 1) * 2);
    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::format("%d.dat", i);
        entry->offset = input_file.stream.read_u32_le();
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        entry->offset = (entry->offset + 0x1F) & (~0x1F);
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = input_file.stream.size() - last_entry->offset;
    return meta;
}

std::unique_ptr<File> Afs2ArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> Afs2ArchiveDecoder::get_linked_formats() const
{
    return {"cri/hca"};
}

static auto dummy = fmt::register_fmt<Afs2ArchiveDecoder>("cri/afs2");
