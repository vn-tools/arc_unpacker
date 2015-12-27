#include "fmt/team_shanghai_alice/pbg3_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_stream.h"

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

static unsigned int read_integer(io::IBitReader &bit_reader)
{
    size_t integer_size = bit_reader.get(2) + 1;
    return bit_reader.get(integer_size << 3);
}

bool Pbg3ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta> Pbg3ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    io::MsbBitReader header_bit_reader(input_file.stream);
    auto file_count = read_integer(header_bit_reader);
    auto table_offset = read_integer(header_bit_reader);

    input_file.stream.seek(table_offset);
    io::MsbBitReader table_bit_reader(input_file.stream);

    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        read_integer(table_bit_reader);
        read_integer(table_bit_reader);
        entry->checksum = read_integer(table_bit_reader);
        entry->offset = read_integer(table_bit_reader);
        entry->size_orig = read_integer(table_bit_reader);
        std::string name;
        for (auto j : algo::range(256))
        {
            char c = table_bit_reader.get(8);
            if (c == 0)
                break;
            name += c;
        }
        entry->path = name;
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size_comp = table_offset - last_entry->offset;
    return meta;
}

std::unique_ptr<io::File> Pbg3ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    io::MsbBitReader bit_reader(input_file.stream.read(entry->size_comp));

    algo::pack::BitwiseLzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    auto data = algo::pack::lzss_decompress(
        bit_reader, entry->size_orig, settings);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Pbg3ArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto dummy
    = fmt::register_fmt<Pbg3ArchiveDecoder>("team-shanghai-alice/pbg3");
