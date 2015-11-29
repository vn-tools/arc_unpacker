#include "fmt/rpgmaker/rgss3a_archive_decoder.h"
#include "fmt/rpgmaker/rgs/common.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "RGSSAD\x00\x03"_b;

bool Rgss3aArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Rgss3aArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    u32 key = input_file.stream.read_u32_le() * 9 + 3;
    auto meta = std::make_unique<ArchiveMeta>();
    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<rgs::ArchiveEntryImpl>();
        entry->offset = input_file.stream.read_u32_le() ^ key;
        if (!entry->offset)
            break;

        entry->size = input_file.stream.read_u32_le() ^ key;
        entry->key = input_file.stream.read_u32_le() ^ key;

        size_t name_size = input_file.stream.read_u32_le() ^ key;
        auto name = input_file.stream.read(name_size).str();
        for (auto i : util::range(name_size))
            name[i] ^= key >> (i << 3);
        entry->path = name;

        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Rgss3aArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    return rgs::read_file_impl(
        input_file, *static_cast<const rgs::ArchiveEntryImpl*>(&e));
}

static auto dummy = fmt::register_fmt<Rgss3aArchiveDecoder>("rpgmaker/rgss3a");
