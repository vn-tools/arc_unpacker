#include "dec/silky/arc_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::silky;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("arc");
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto table_size = input_file.stream.read_le<u32>();
    io::MemoryStream table_stream(input_file.stream.read(table_size));

    auto meta = std::make_unique<ArchiveMeta>();
    while (!table_stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto name_size = table_stream.read<u8>();
        auto name = table_stream.read(name_size);
        for (const auto j : algo::range(name.size()))
            name[j] += name.size() - j;
        entry->path = algo::sjis_to_utf8(name).str();
        entry->size_comp = table_stream.read_be<u32>();
        entry->size_orig = table_stream.read_be<u32>();
        entry->offset = table_stream.read_be<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->size_comp != entry->size_orig)
        data = algo::pack::lzss_decompress(data, entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"silky/akb"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("silky/arc");
