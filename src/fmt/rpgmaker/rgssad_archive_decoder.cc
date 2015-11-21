#include "fmt/rpgmaker/rgssad_archive_decoder.h"
#include "fmt/rpgmaker/rgs/common.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "RGSSAD\x00\x01"_b;
static const u32 initial_key = 0xDEADCAFE;

bool RgssadArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    RgssadArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(magic.size());
    auto key = initial_key;
    auto meta = std::make_unique<ArchiveMeta>();
    while (!arc_file.stream.eof())
    {
        auto entry = std::make_unique<rgs::ArchiveEntryImpl>();

        size_t name_size = arc_file.stream.read_u32_le() ^ key;
        key = rgs::advance_key(key);
        entry->name = arc_file.stream.read(name_size).str();
        for (auto i : util::range(name_size))
        {
            entry->name[i] ^= key;
            key = rgs::advance_key(key);
        }

        entry->size = arc_file.stream.read_u32_le() ^ key;
        key = rgs::advance_key(key);

        entry->key = key;
        entry->offset = arc_file.stream.tell();

        arc_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> RgssadArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    return rgs::read_file_impl(
        arc_file, *static_cast<const rgs::ArchiveEntryImpl*>(&e));
}

static auto dummy = fmt::register_fmt<RgssadArchiveDecoder>("rpgmaker/rgssad");
