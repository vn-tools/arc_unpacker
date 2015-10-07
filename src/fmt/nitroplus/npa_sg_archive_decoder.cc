#include "fmt/nitroplus/npa_sg_archive_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

static const bstr key = "\xBD\xAA\xBC\xB4\xAB\xB6\xBC\xB4"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(bstr &data)
{
    for (auto i : util::range(data.size()))
        data[i] ^= key[i % key.size()];
}

bool NpaSgArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    if (!arc_file.has_extension("npa"))
        return false;
    size_t table_size = arc_file.io.read_u32_le();
    return table_size < arc_file.io.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    NpaSgArchiveDecoder::read_meta(File &arc_file) const
{
    size_t table_size = arc_file.io.read_u32_le();
    auto table_data = arc_file.io.read(table_size);
    decrypt(table_data);
    io::BufferedIO table_io(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    size_t file_count = table_io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::convert_encoding(
            table_io.read(table_io.read_u32_le()), "utf-16le", "utf-8").str();
        entry->size = table_io.read_u32_le();
        entry->offset = table_io.read_u32_le();
        table_io.skip(4);
        if (entry->offset + entry->size > arc_file.io.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> NpaSgArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    decrypt(data);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<NpaSgArchiveDecoder>("nitro/npa-sg");
