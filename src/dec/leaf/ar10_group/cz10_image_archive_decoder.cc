#include "dec/leaf/ar10_group/cz10_image_archive_decoder.h"
#include <array>
#include "algo/format.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "cz10"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset, size;
        size_t width, height, channels;
    };
}

static bstr decompress(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    const auto stride = width * channels;

    bstr last_line(stride);
    bstr output;
    output.reserve(stride * height);

    io::MemoryStream input_stream(input);
    for (const auto y : algo::range(height))
    {
        const auto control = input_stream.read_u8();
        bstr line;

        if (control == 0)
        {
            line = input_stream.read(stride);
        }
        else if (control == 1)
        {
            const auto chunk_size = input_stream.read_u16_be();
            const auto chunk = input_stream.read(chunk_size);
            line = algo::pack::zlib_inflate(chunk);
        }
        else if (control == 2)
        {
            const auto chunk_size = input_stream.read_u16_be();
            const auto chunk = input_stream.read(chunk_size);
            line = algo::pack::zlib_inflate(chunk);
            for (const auto i : algo::range(1, stride))
                line[i] = line[i - 1] - line[i];
        }
        else if (control == 3)
        {
            const auto chunk_size = input_stream.read_u16_be();
            const auto chunk = input_stream.read(chunk_size);
            line = algo::pack::zlib_inflate(chunk);
            for (const auto i : algo::range(stride))
                line[i] = last_line[i] - line[i];
        }
        else
            throw err::CorruptDataError("Unexpected control byte");

        output += line;
        last_line = line;
    }

    return output;
}

bool Cz10ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Cz10ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();

    input_file.stream.seek(magic.size());
    const auto image_count = input_file.stream.read_u32_le();
    auto current_offset = input_file.stream.tell() + image_count * 16;
    for (const auto i : algo::range(image_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->width = input_file.stream.read_u16_le();
        entry->height = input_file.stream.read_u16_le();
        input_file.stream.skip(4);
        entry->size = input_file.stream.read_u32_le();
        entry->channels = input_file.stream.read_u32_le();
        entry->offset = current_offset;
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<io::File> Cz10ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = decompress(
        input_file.stream.seek(entry->offset).read(entry->size),
        entry->width,
        entry->height,
        entry->channels);

    if (entry->channels != 4)
        throw err::UnsupportedChannelCountError(entry->channels);

    res::Image image(entry->width, entry->height);
    const auto *data_ptr = data.get<const u8>();
    for (const auto y : algo::range(entry->height))
    for (const auto c : algo::range(entry->channels))
    for (const auto x : algo::range(entry->width))
    {
        image.at(x, y)[c] = *data_ptr++;
    }
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

dec::NamingStrategy Cz10ImageArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<Cz10ImageArchiveDecoder>("leaf/cz10");
