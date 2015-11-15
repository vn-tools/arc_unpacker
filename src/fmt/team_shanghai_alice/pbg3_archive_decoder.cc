#include "fmt/team_shanghai_alice/pbg3_archive_decoder.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static const bstr magic = "PBG3"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        u32 checksum;
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

static unsigned int read_integer(io::BitReader &bit_reader)
{
    size_t integer_size = bit_reader.get(2) + 1;
    return bit_reader.get(integer_size << 3);
}

bool Pbg3ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Pbg3ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());

    io::BitReader header_bit_reader(arc_file.io);
    auto file_count = read_integer(header_bit_reader);
    auto table_offset = read_integer(header_bit_reader);

    arc_file.io.seek(table_offset);
    io::BitReader table_bit_reader(arc_file.io);

    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        read_integer(table_bit_reader);
        read_integer(table_bit_reader);
        entry->checksum = read_integer(table_bit_reader);
        entry->offset = read_integer(table_bit_reader);
        entry->size_orig = read_integer(table_bit_reader);
        for (auto j : util::range(256))
        {
            char c = table_bit_reader.get(8);
            if (c == 0)
                break;
            entry->name.push_back(c);
        }
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size_comp = table_offset - last_entry->offset;
    return meta;
}

std::unique_ptr<File> Pbg3ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    io::BitReader bit_reader(arc_file.io.read(entry->size_comp));

    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    auto data = util::pack::lzss_decompress_bitwise(
        bit_reader, entry->size_orig, settings);

    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> Pbg3ArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto dummy
    = fmt::register_fmt<Pbg3ArchiveDecoder>("team-shanghai-alice/pbg3");
