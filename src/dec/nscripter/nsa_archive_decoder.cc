#include "dec/nscripter/nsa_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/nscripter/spb_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::nscripter;

namespace
{
    enum CompressionType
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_SPB = 1,
        COMPRESSION_LZSS = 2,
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        CompressionType compression_type;
    };
}

bool NsaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto file_count = input_file.stream.read_be<u16>();
    const auto offset_to_files = input_file.stream.read_be<u32>();
    if (file_count == 0)
        return false;
    for (const auto i : algo::range(file_count))
    {
        input_file.stream.read_to_zero();
        input_file.stream.read<u8>();
        const auto offset = input_file.stream.read_be<u32>();
        const auto size_comp = input_file.stream.read_be<u32>();
        const auto size_orig = input_file.stream.read_be<u32>();
        if (offset_to_files + offset + size_comp > input_file.stream.size())
            return false;
    }
    return true;
}

std::unique_ptr<dec::ArchiveMeta> NsaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    const auto file_count = input_file.stream.read_be<u16>();
    const auto offset_to_data = input_file.stream.read_be<u32>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = input_file.stream.read_to_zero().str();
        entry->compression_type =
            static_cast<CompressionType>(input_file.stream.read<u8>());
        entry->offset = input_file.stream.read_be<u32>() + offset_to_data;
        entry->size_comp = input_file.stream.read_be<u32>();
        entry->size_orig = input_file.stream.read_be<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> NsaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    const auto data = input_file.stream
        .seek(entry->offset)
        .read(entry->size_comp);

    if (entry->compression_type == COMPRESSION_NONE)
        return std::make_unique<io::File>(entry->path, data);

    if (entry->compression_type == COMPRESSION_LZSS)
    {
        algo::pack::BitwiseLzssSettings settings;
        settings.position_bits = 8;
        settings.size_bits = 4;
        settings.min_match_size = 2;
        settings.initial_dictionary_pos = 239;
        return std::make_unique<io::File>(
            entry->path,
            algo::pack::lzss_decompress(data, entry->size_orig, settings));
    }

    if (entry->compression_type == COMPRESSION_SPB)
    {
        const auto decoder = SpbImageDecoder();
        const auto encoder = enc::png::PngImageEncoder();
        io::File spb_file("dummy.spb", data);
        return encoder.encode(
            logger, decoder.decode(logger, spb_file), entry->path);
    }

    throw err::NotSupportedError("Unknown compression type");
}

static auto _ = dec::register_decoder<NsaArchiveDecoder>("nscripter/nsa");
