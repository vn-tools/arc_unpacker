#include "fmt/touhou/pbg4_archive_decoder.h"
#include "fmt/touhou/anm_archive_decoder.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

static const bstr magic = "PBG4"_b;

static bstr decompress(const bstr &data, size_t size_orig)
{
    util::pack::LzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    return util::pack::lzss_decompress_bitwise(data, size_orig, settings);
}

struct Pbg4ArchiveDecoder::Priv final
{
    AnmArchiveDecoder anm_archive_decoder;
};

Pbg4ArchiveDecoder::Pbg4ArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->anm_archive_decoder);
}

Pbg4ArchiveDecoder::~Pbg4ArchiveDecoder()
{
}

bool Pbg4ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Pbg4ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_count = arc_file.io.read_u32_le();
    auto table_offset = arc_file.io.read_u32_le();
    auto table_size_orig = arc_file.io.read_u32_le();

    arc_file.io.seek(table_offset);
    auto table_data = arc_file.io.read_to_eof();
    table_data = decompress(table_data, table_size_orig);
    io::BufferedIO table_io(table_data);

    ArchiveEntryImpl *last_entry = nullptr;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read_to_zero().str();
        entry->offset = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        table_io.skip(4);
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size_comp = arc_file.io.size() - last_entry->offset;
    return meta;
}

std::unique_ptr<File> Pbg4ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);
    data = decompress(data, entry->size_orig);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<Pbg4ArchiveDecoder>("th/pbg4");
