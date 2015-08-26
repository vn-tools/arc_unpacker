// NSA archive
//
// Company:   -
// Engine:    NScripter
// Extension: .nsa
//
// Known games:
// - Tsukihime
// - Umineko no naku koro ni

#include "fmt/nscripter/nsa_archive.h"
#include "fmt/nscripter/spb_converter.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nscripter;

namespace
{
    enum CompressionType
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_SPB = 1,
        COMPRESSION_LZSS = 2,
    };

    struct TableEntry
    {
        std::string name;
        CompressionType compression_type;
        size_t offset;
        size_t size_compressed;
        size_t size_original;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    size_t file_count = arc_io.read_u16_be();
    size_t offset_to_files = arc_io.read_u32_be();
    if (offset_to_files > arc_io.size())
        throw std::runtime_error("Bad offset to files");

    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read_to_zero().str();
        entry->compression_type =
            static_cast<CompressionType>(arc_io.read_u8());
        entry->offset = arc_io.read_u32_be();
        entry->size_compressed = arc_io.read_u32_be();
        entry->size_original = arc_io.read_u32_be();

        entry->offset += offset_to_files;

        if (entry->offset + entry->size_compressed > arc_io.size())
            throw std::runtime_error("Bad offset to file");

        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, SpbConverter &spb_converter)
{
    std::unique_ptr<File> file(new File);

    file->name = entry.name;
    arc_io.seek(entry.offset);
    bstr data = arc_io.read(entry.size_compressed);

    switch (entry.compression_type)
    {
        case COMPRESSION_NONE:
            file->io.write(data);
            break;

        case COMPRESSION_LZSS:
        {
            util::pack::LzssSettings settings;
            settings.position_bits = 8;
            settings.size_bits = 4;
            settings.min_match_size = 2;
            settings.initial_dictionary_pos = 239;
            file->io.write(util::pack::lzss_decompress_bitwise(
                data, entry.size_original, settings));
            break;
        }

        case COMPRESSION_SPB:
            file->io.write(data);
            file = spb_converter.decode(*file);
            break;
    }

    return file;
}

struct NsaArchive::Priv
{
    SpbConverter spb_converter;
};

NsaArchive::NsaArchive() : p(new Priv)
{
}

NsaArchive::~NsaArchive()
{
}

bool NsaArchive::is_recognized_internal(File &arc_file) const
{
    size_t file_count = arc_file.io.read_u16_be();
    size_t offset_to_files = arc_file.io.read_u32_be();
    if (file_count == 0)
        return false;
    for (auto i : util::range(file_count))
    {
        arc_file.io.read_to_zero();
        arc_file.io.read_u8();
        size_t offset = arc_file.io.read_u32_be();
        size_t size_compressed = arc_file.io.read_u32_be();
        size_t size_original = arc_file.io.read_u32_be();
        if (offset_to_files + offset + size_compressed > arc_file.io.size())
            return false;
    }
    return true;
}

void NsaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry, p->spb_converter));
}
