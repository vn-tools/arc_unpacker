#include "fmt/innocent_grey/packdat_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::fmt::innocent_grey;

static const bstr magic = "PACKDAT\x2E"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_orig;
        size_t size_comp;
        bool encrypted;
    };
}

bool PackdatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PackdatArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(32).str();
        entry->offset = input_file.stream.read_u32_le();
        entry->encrypted = input_file.stream.read_u32_le() & 0x10000;
        entry->size_orig = input_file.stream.read_u32_le();
        entry->size_comp = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PackdatArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);
    if (entry->size_comp != entry->size_orig)
        throw err::NotSupportedError("Compressed files are not supported");
    if (entry->encrypted)
    {
        auto data_ptr = data.get<u32>();
        const auto data_end = data.end<const u32>();
        const auto size = data.size() >> 2;
        u32 key = (size << ((size & 7) + 8)) ^ size;
        while (data_ptr < data_end)
        {
            const auto tmp = *data_ptr ^ key;
            const auto rot = tmp % 24;
            *data_ptr++ = tmp;
            key = (key << rot) | (key >> (32 - rot));
        }
    }
    return std::make_unique<io::File>(entry->path, data);
}

static auto dummy
    = fmt::register_fmt<PackdatArchiveDecoder>("innocent-grey/packdat");
