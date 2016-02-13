#include "dec/rpgmaker/rgss3a_archive_decoder.h"
#include "algo/range.h"
#include "dec/rpgmaker/rgs/common.h"

using namespace au;
using namespace au::dec::rpgmaker;

static const bstr magic = "RGSSAD\x00\x03"_b;

bool Rgss3aArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Rgss3aArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    u32 key = input_file.stream.read_le<u32>() * 9 + 3;
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<rgs::CustomArchiveEntry>();
        entry->offset = input_file.stream.read_le<u32>() ^ key;
        if (!entry->offset)
            break;

        entry->size = input_file.stream.read_le<u32>() ^ key;
        entry->key = input_file.stream.read_le<u32>() ^ key;

        const auto name_size = input_file.stream.read_le<u32>() ^ key;
        auto name = input_file.stream.read(name_size).str();
        for (const auto i : algo::range(name_size))
            name[i] ^= key >> (i << 3);
        entry->path = name;

        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Rgss3aArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    return rgs::read_file_impl(
        input_file, *static_cast<const rgs::CustomArchiveEntry*>(&e));
}

static auto _ = dec::register_decoder<Rgss3aArchiveDecoder>("rpgmaker/rgss3a");
