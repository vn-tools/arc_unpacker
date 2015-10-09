#include "fmt/alice_soft/alk_archive_decoder.h"
#include "fmt/alice_soft/qnt_image_decoder.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "ALK0"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct AlkArchiveDecoder::Priv final
{
    QntImageDecoder qnt_image_decoder;
};

AlkArchiveDecoder::AlkArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->qnt_image_decoder);
}

AlkArchiveDecoder::~AlkArchiveDecoder()
{
}

bool AlkArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    AlkArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        if (entry->size)
        {
            entry->name = util::format("%03d.dat", meta->entries.size());
            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

std::unique_ptr<File> AlkArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<AlkArchiveDecoder>("alice/alk");
