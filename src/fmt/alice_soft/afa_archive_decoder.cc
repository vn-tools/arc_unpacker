#include "fmt/alice_soft/afa_archive_decoder.h"
#include "err.h"
#include "fmt/alice_soft/aff_file_decoder.h"
#include "fmt/alice_soft/ajp_image_decoder.h"
#include "fmt/alice_soft/qnt_image_decoder.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic1 = "AFAH"_b;
static const bstr magic2 = "AlicArch"_b;
static const bstr magic3 = "INFO"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct AfaArchiveDecoder::Priv final
{
    AffFileDecoder aff_file_decoder;
    AjpImageDecoder ajp_image_decoder;
    QntImageDecoder qnt_image_decoder;
};

AfaArchiveDecoder::AfaArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->aff_file_decoder);
    add_decoder(&p->ajp_image_decoder);
    add_decoder(&p->qnt_image_decoder);
}

AfaArchiveDecoder::~AfaArchiveDecoder()
{
}

bool AfaArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (arc_file.io.read(magic1.size()) != magic1)
        return false;
    arc_file.io.skip(4);
    return arc_file.io.read(magic2.size()) == magic2;
}

std::unique_ptr<fmt::ArchiveMeta>
    AfaArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic1.size() + 4 + magic2.size() + 4 * 2);
    auto file_data_start = arc_file.io.read_u32_le();
    if (arc_file.io.read(magic3.size()) != magic3)
        throw err::CorruptDataError("Corrupt file table");

    auto table_size_compressed = arc_file.io.read_u32_le();
    auto table_size_original = arc_file.io.read_u32_le();
    auto file_count = arc_file.io.read_u32_le();

    io::BufferedIO table_io(
        util::pack::zlib_inflate(
            arc_file.io.read(table_size_compressed)));

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        table_io.skip(4);
        auto name_size = table_io.read_u32_le();
        entry->name = util::sjis_to_utf8(
            table_io.read_to_zero(name_size)).str();

        table_io.skip(4 * 3); // for some games, apparently this is 4 * 2
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size = table_io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> AfaArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<AfaArchiveDecoder>("alice/afa");
