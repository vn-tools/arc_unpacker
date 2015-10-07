#include "fmt/leaf/leafpack_archive_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAFPACK"_b;
static const bstr key = "\x51\x42\xFE\x77\x2D\x65\x48\x7E\x0A\x8A\xE5"_b;

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
        data[i] -= key[i % key.size()];
}

bool LeafpackArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    LeafpackArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u16_le();

    auto table_size = file_count * 24;
    arc_file.io.seek(arc_file.io.size() - table_size);
    auto table_data = arc_file.io.read(table_size);
    decrypt(table_data);

    io::BufferedIO table_io(table_data);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read_to_zero(12).str();
        auto space_pos = entry->name.find_first_of(' ');
        if (space_pos != std::string::npos)
        {
            entry->name[space_pos] = '.';
            while (entry->name[space_pos + 1] == ' ')
                entry->name.erase(space_pos + 1, 1);
        }
        entry->offset = table_io.read_u32_le();
        entry->size = table_io.read_u32_le();
        table_io.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> LeafpackArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    decrypt(data);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<LeafpackArchiveDecoder>("leaf/leafpack");
