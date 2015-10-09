#include "fmt/propeller/mpk_archive_decoder.h"
#include "fmt/propeller/mgr_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::propeller;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct MpkArchiveDecoder::Priv final
{
    MgrArchiveDecoder mgr_archive_decoder;
};

MpkArchiveDecoder::MpkArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->mgr_archive_decoder);
}

MpkArchiveDecoder::~MpkArchiveDecoder()
{
}

bool MpkArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("mpk");
}

std::unique_ptr<fmt::ArchiveMeta>
    MpkArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto table_offset = arc_file.io.read_u32_le();
    auto file_count = arc_file.io.read_u32_le();

    arc_file.io.seek(table_offset);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        auto name = arc_file.io.read(32);
        u8 key8 = name[31];
        u32 key32 = (key8 << 24) | (key8 << 16) | (key8 << 8) | key8;

        for (auto i : util::range(32))
            name[i] ^= key8;

        entry->name = util::sjis_to_utf8(name).str();
        if (entry->name[0] == '\\')
            entry->name = entry->name.substr(1);
        entry->name.erase(entry->name.find('\x00'));

        entry->offset = arc_file.io.read_u32_le() ^ key32;
        entry->size = arc_file.io.read_u32_le() ^ key32;

        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> MpkArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<MpkArchiveDecoder>("propeller/mpk");
