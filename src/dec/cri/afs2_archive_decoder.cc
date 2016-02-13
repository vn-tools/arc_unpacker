#include "dec/cri/afs2_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::cri;

static const bstr magic = "AFS2"_b;

bool Afs2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Afs2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto file_count = input_file.stream.read_le<u32>() - 1;
    input_file.stream.skip(4);
    input_file.stream.skip((file_count + 1) * 2);
    PlainArchiveEntry *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::format("%d.dat", i);
        entry->offset = input_file.stream.read_le<u32>();
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

std::unique_ptr<io::File> Afs2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Afs2ArchiveDecoder::get_linked_formats() const
{
    return {"cri/hca"};
}

static auto _ = dec::register_decoder<Afs2ArchiveDecoder>("cri/afs2");
