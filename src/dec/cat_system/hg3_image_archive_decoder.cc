#include "dec/cat_system/hg3_image_archive_decoder.h"
#include <map>
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "dec/jpeg/jpeg_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/lsb_bit_reader.h"
#include "io/memory_stream.h"
#include "ptr.h"

using namespace au;
using namespace au::dec::cat_system;

static const bstr magic = "HG-3"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static u8 convert_value(u8 value)
{
    const bool carry = (value & 1) != 0;
    value >>= 1;
    return carry ? value ^ 0xFF : value;
}

static bstr delta_transform(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    u32 table[4][256];
    for (const auto i : algo::range(256))
    {
        u32 value = 0;
        value = (value << 6) | (i & 0b11000000);
        value = (value << 6) | (i & 0b00110000);
        value = (value << 6) | (i & 0b00001100);
        value = (value << 6) | (i & 0b00000011);
        table[0][i] = value << 6;
        table[1][i] = value << 4;
        table[2][i] = value << 2;
        table[3][i] = value;
    }

    const auto plane_size = input.size() / 4;
    auto plane0 = 0;
    auto plane1 = plane0 + plane_size;
    auto plane2 = plane1 + plane_size;
    auto plane3 = plane2 + plane_size;

    bstr output(input.size());
    auto output_ptr = make_ptr(output);
    while (output_ptr < output_ptr.end())
    {
        u32 value
            = table[0][input[plane0++]]
            | table[1][input[plane1++]]
            | table[2][input[plane2++]]
            | table[3][input[plane3++]];

        *output_ptr++ = convert_value(value);
        *output_ptr++ = convert_value(value >> 8);
        *output_ptr++ = convert_value(value >> 16);
        *output_ptr++ = convert_value(value >> 24);
    }

    const auto channels = depth >> 3;
    const auto stride = width * channels;

    output_ptr = make_ptr(output) + channels;
    for (const auto x : algo::range(stride - channels))
    {
        *output_ptr += output_ptr[-channels];
        output_ptr++;
    }

    output_ptr = make_ptr(output) + stride;
    for (const auto y : algo::range(1, height))
    for (const auto x : algo::range(stride))
    {
        *output_ptr += output_ptr[-stride];
        output_ptr++;
    }

    return output;
}

static std::unique_ptr<res::Image> decode_img0000(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t depth)
{
    io::MemoryStream input_stream(input);
    input_stream.seek(8);
    const auto data_size_comp = input_stream.read_u32_le();
    const auto data_size_orig = input_stream.read_u32_le();
    const auto ctl_size_comp = input_stream.read_u32_le();
    const auto ctl_size_orig = input_stream.read_u32_le();
    const auto data = algo::pack::zlib_inflate(
        input_stream.read(data_size_comp));
    const auto ctl = algo::pack::zlib_inflate(input_stream.read(ctl_size_comp));

    io::LsbBitReader ctl_bit_reader(ctl);
    bool copy = ctl_bit_reader.get(1);
    const auto output_size = ctl_bit_reader.get_gamma(1);

    bstr output(output_size);
    auto input_ptr = make_ptr(data);
    auto output_ptr = make_ptr(output);
    while (output_ptr < output_ptr.end())
    {
        auto size = ctl_bit_reader.get_gamma(1);
        if (copy)
        {
            while (size--
                && input_ptr < input_ptr.end()
                && output_ptr < output_ptr.end())
            {
                *output_ptr++ = *input_ptr++;
            }
        }
        else
        {
            output_ptr += size;
        }
        copy = !copy;
    }

    output = delta_transform(output, width, height, depth);
    auto image = std::make_unique<res::Image>(
        width, height, output, res::PixelFormat::BGRA8888);
    image->flip_vertically();
    return image;
}

static std::unique_ptr<res::Image> decode_jpeg(
    const Logger &logger, const bstr &input)
{
    // const auto jpeg_decoder = dec::jpeg::JpegImageDecoder();
    // auto pseudo_file = std::make_unique<io::File>("dummy.jpg", input);
    // return std::make_unique<res::Image>(
    //     jpeg_decoder.decode(logger, *pseudo_file));
    throw err::NotSupportedError("Not implemented");
}

bool Hg3ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Hg3ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(12);
    auto meta = std::make_unique<ArchiveMeta>();
    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = input_file.stream.tell() + 8;
        entry->size = input_file.stream.read_u32_le();
        input_file.stream.skip(4);
        if (!entry->size)
            entry->size = input_file.stream.size() - entry->offset;
        else
            entry->size -= 8;
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Hg3ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    io::MemoryStream data_stream(data);

    std::map<bstr, bstr> chunks;
    while (!data_stream.eof())
    {
        const auto chunk_name = data_stream.read(8);
        data_stream.skip(4);
        const auto chunk_size = data_stream.read_u32_le();
        const auto chunk_data = data_stream.read(chunk_size);
        chunks[chunk_name] = chunk_data;
    }

    io::MemoryStream header_stream(chunks.at("stdinfo\x00"_b));
    const auto width = header_stream.read_u32_le();
    const auto height = header_stream.read_u32_le();
    const auto depth = header_stream.read_u32_le();
    const auto x = header_stream.read_u32_le();
    const auto y = header_stream.read_u32_le();
    const auto canvas_width = header_stream.read_u32_le();
    const auto canvas_height = header_stream.read_u32_le();

    std::unique_ptr<res::Image> image;
    if (chunks.find("img0000\x00"_b) != chunks.end())
        image = decode_img0000(chunks["img0000\x00"_b], width, height, depth);
    else if (chunks.find("img_jpg\x00"_b) != chunks.end())
        image = decode_jpeg(logger, chunks["img_jpg\x00"_b]);
    else
        throw err::NotSupportedError("No image data found!\n");

    res::Image actual_image(canvas_width, canvas_height);
    actual_image.paste(*image, x, y);
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, actual_image, entry->path);
}

dec::NamingStrategy Hg3ImageArchiveDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<Hg3ImageArchiveDecoder>("cat-system/hg3");
