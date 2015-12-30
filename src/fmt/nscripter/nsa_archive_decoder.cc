#include "fmt/nscripter/nsa_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "fmt/nscripter/spb_image_decoder.h"
#include "io/memory_stream.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::fmt::nscripter;

namespace
{
    enum CompressionType
    {
        COMPRESSStreamN_NONE = 0,
        COMPRESSStreamN_SPB = 1,
        COMPRESSStreamN_LZSS = 2,
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        CompressionType compression_type;
        size_t offset;
        size_t size_comp;
        size_t size_orig;
    };
}

bool NsaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    size_t file_count = input_file.stream.read_u16_be();
    size_t offset_to_files = input_file.stream.read_u32_be();
    if (file_count == 0)
        return false;
    for (auto i : algo::range(file_count))
    {
        input_file.stream.read_to_zero();
        input_file.stream.read_u8();
        size_t offset = input_file.stream.read_u32_be();
        size_t size_comp = input_file.stream.read_u32_be();
        size_t size_orig = input_file.stream.read_u32_be();
        if (offset_to_files + offset + size_comp > input_file.stream.size())
            return false;
    }
    return true;
}

std::unique_ptr<fmt::ArchiveMeta> NsaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    size_t file_count = input_file.stream.read_u16_be();
    size_t offset_to_data = input_file.stream.read_u32_be();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero().str();
        entry->compression_type =
            static_cast<CompressionType>(input_file.stream.read_u8());
        entry->offset = input_file.stream.read_u32_be() + offset_to_data;
        entry->size_comp = input_file.stream.read_u32_be();
        entry->size_orig = input_file.stream.read_u32_be();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> NsaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<io::File>();

    output_file->path = entry->path;
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size_comp);

    switch (entry->compression_type)
    {
        case COMPRESSStreamN_NONE:
            output_file->stream.write(data);
            break;

        case COMPRESSStreamN_LZSS:
        {
            algo::pack::BitwiseLzssSettings settings;
            settings.position_bits = 8;
            settings.size_bits = 4;
            settings.min_match_size = 2;
            settings.initial_dictionary_pos = 239;
            output_file->stream.write(algo::pack::lzss_decompress(
                data, entry->size_orig, settings));
            break;
        }

        case COMPRESSStreamN_SPB:
        {
            const auto spb_image_decoder = SpbImageDecoder();
            output_file->stream.write(data);
            output_file = util::file_from_image(
                spb_image_decoder.decode(logger, *output_file),
                output_file->path);
            break;
        }
    }

    return output_file;
}

static auto dummy = fmt::register_fmt<NsaArchiveDecoder>("nscripter/nsa");
