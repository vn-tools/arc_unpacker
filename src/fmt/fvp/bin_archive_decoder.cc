#include "fmt/fvp/bin_archive_decoder.h"
#include "fmt/fvp/nvsg_image_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fvp;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct BinArchiveDecoder::Priv final
{
    NvsgImageDecoder nvsg_image_decoder;
};

BinArchiveDecoder::BinArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->nvsg_image_decoder);
}

BinArchiveDecoder::~BinArchiveDecoder()
{
}

bool BinArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.has_extension("bin");
}

std::unique_ptr<fmt::ArchiveMeta>
    BinArchiveDecoder::read_meta_impl(File &arc_file) const
{
    size_t file_count = arc_file.io.read_u32_le();
    arc_file.io.skip(4);

    auto names_start = file_count * 12 + 8;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name_offset = arc_file.io.read_u32_le();
        arc_file.io.peek(names_start + name_offset, [&]()
        {
            entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero()).str();
        });
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> BinArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<BinArchiveDecoder>("fvp/bin");
