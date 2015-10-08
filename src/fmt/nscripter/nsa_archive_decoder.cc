#include "fmt/nscripter/nsa_archive_decoder.h"
#include "fmt/nscripter/spb_image_decoder.h"
#include "io/buffered_io.h"
#include "util/file_from_grid.h"
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

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        CompressionType compression_type;
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

bool NsaArchiveDecoder::is_recognized_internal(File &arc_file) const
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
        size_t size_comp = arc_file.io.read_u32_be();
        size_t size_orig = arc_file.io.read_u32_be();
        if (offset_to_files + offset + size_comp > arc_file.io.size())
            return false;
    }
    return true;
}

std::unique_ptr<fmt::ArchiveMeta>
    NsaArchiveDecoder::read_meta(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    size_t file_count = arc_file.io.read_u16_be();
    size_t offset_to_data = arc_file.io.read_u32_be();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero().str();
        entry->compression_type =
            static_cast<CompressionType>(arc_file.io.read_u8());
        entry->offset = arc_file.io.read_u32_be() + offset_to_data;
        entry->size_comp = arc_file.io.read_u32_be();
        entry->size_orig = arc_file.io.read_u32_be();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> NsaArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<File>();

    output_file->name = entry->name;
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);

    switch (entry->compression_type)
    {
        case COMPRESSION_NONE:
            output_file->io.write(data);
            break;

        case COMPRESSION_LZSS:
        {
            util::pack::LzssSettings settings;
            settings.position_bits = 8;
            settings.size_bits = 4;
            settings.min_match_size = 2;
            settings.initial_dictionary_pos = 239;
            output_file->io.write(util::pack::lzss_decompress_bitwise(
                data, entry->size_orig, settings));
            break;
        }

        case COMPRESSION_SPB:
        {
            SpbImageDecoder spb_image_decoder;
            output_file->io.write(data);
            output_file = util::file_from_grid(
                spb_image_decoder.decode(*output_file), output_file->name);
            break;
        }
    }

    return output_file;
}

static auto dummy = fmt::Registry::add<NsaArchiveDecoder>("nscripter/nsa");
