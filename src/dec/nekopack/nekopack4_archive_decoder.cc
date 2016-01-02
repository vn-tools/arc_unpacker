#include "dec/nekopack/nekopack4_archive_decoder.h"
#include <map>
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::nekopack;

static const bstr magic = "NEKOPACK4A"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        u32 offset;
        u32 size_comp;
    };
}

bool Nekopack4ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Nekopack4ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto table_size = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        const auto name_size = input_file.stream.read_u32_le();
        if (!name_size)
            break;

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(name_size).str();
        u32 key = 0;
        for (const u8 &c : entry->path.str())
            key += c;
        entry->offset = input_file.stream.read_u32_le() ^ key;
        entry->size_comp = input_file.stream.read_u32_le() ^ key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Nekopack4ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream
        .seek(entry->offset)
        .read(entry->size_comp - 4);
    const auto size_orig = input_file.stream.read_u32_le();

    u8 key = (entry->size_comp >> 3) + 0x22;
    for (auto &c : data)
    {
        c ^= key;
        key <<= 3;
        if (!key)
            break;
    }

    data = algo::pack::zlib_inflate(data);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Nekopack4ArchiveDecoder::get_linked_formats() const
{
    return {"nekopack/masked-bmp"};
}

static auto _ = dec::register_decoder<Nekopack4ArchiveDecoder>(
    "nekopack/nekopack4");
