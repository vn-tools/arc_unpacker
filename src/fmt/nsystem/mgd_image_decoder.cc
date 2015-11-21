#include "fmt/nsystem/mgd_image_decoder.h"
#include "err.h"
#include "fmt/png/png_image_decoder.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nsystem;

namespace
{
    enum class CompressionType : u8
    {
        None = 0,
        Sgd = 1,
        Png = 2,
    };

    struct Region final
    {
        u16 x;
        u16 y;
        u16 width;
        u16 height;
    };
}

static const bstr magic = "MGD "_b;

static void decompress_sgd_alpha(const bstr &input, io::Stream &output_stream)
{
    io::MemoryStream input_stream(input);
    while (!input_stream.eof())
    {
        auto flag = input_stream.read_u16_le();
        if (flag & 0x8000)
        {
            size_t size = (flag & 0x7FFF) + 1;
            u8 alpha = input_stream.read_u8();
            for (auto i : util::range(size))
            {
                output_stream.skip(3);
                output_stream.write_u8(alpha ^ 0xFF);
            }
        }
        else
        {
            while (flag-- && !input_stream.eof())
            {
                u8 alpha = input_stream.read_u8();
                output_stream.skip(3);
                output_stream.write_u8(alpha ^ 0xFF);
            }
        }
    }
    output_stream.seek(0);
}

static void decompress_sgd_bgr_strategy_1(
    io::Stream &input_stream, io::Stream &output_stream, u8 flag)
{
    auto size = flag & 0x3F;
    output_stream.skip(-4);
    u8 b = output_stream.read_u8();
    u8 g = output_stream.read_u8();
    u8 r = output_stream.read_u8();
    output_stream.skip(1);
    for (auto i : util::range(size))
    {
        u16 delta = input_stream.read_u16_le();
        if (delta & 0x8000)
        {
            b += delta & 0x1F;
            g += (delta >> 5) & 0x1F;
            r += (delta >> 10) & 0x1F;
        }
        else
        {
            b += ( delta        & 0xF) * (delta &   0x10 ? -1 : 1);
            g += ((delta >>  5) & 0xF) * (delta &  0x200 ? -1 : 1);
            r += ((delta >> 10) & 0xF) * (delta & 0x4000 ? -1 : 1);
        }

        output_stream.write_u8(b);
        output_stream.write_u8(g);
        output_stream.write_u8(r);
        output_stream.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_2(
    io::Stream &input_stream, io::Stream &output_stream, u8 flag)
{
    auto size = (flag & 0x3F) + 1;
    u8 b = input_stream.read_u8();
    u8 g = input_stream.read_u8();
    u8 r = input_stream.read_u8();
    for (auto i : util::range(size))
    {
        output_stream.write_u8(b);
        output_stream.write_u8(g);
        output_stream.write_u8(r);
        output_stream.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_3(
    io::Stream &input_stream, io::Stream &output_stream, u8 flag)
{
    auto size = flag;
    for (auto i : util::range(size))
    {
        output_stream.write(input_stream.read(3));
        output_stream.skip(1);
    }
}

static void decompress_sgd_bgr(const bstr &input, io::Stream &output_stream)
{
    io::MemoryStream input_stream(input);
    while (!input_stream.eof())
    {
        u8 flag = input_stream.read_u8();
        std::function<void(io::Stream &, io::Stream &, u8)> func;
        switch (flag & 0xC0)
        {
            case 0x80: func = decompress_sgd_bgr_strategy_1; break;
            case 0x40: func = decompress_sgd_bgr_strategy_2; break;
            case 0x00: func = decompress_sgd_bgr_strategy_3; break;
            default: throw err::CorruptDataError("Bad decompression flag");
        }
        func(input_stream, output_stream, flag);
    }
    output_stream.seek(0);
}

static bstr decompress_sgd(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    io::MemoryStream output_stream(output);

    io::MemoryStream tmp_stream(input);

    auto alpha_size = tmp_stream.read_u32_le();
    auto alpha_data = tmp_stream.read(alpha_size);
    decompress_sgd_alpha(alpha_data, output_stream);

    auto color_size = tmp_stream.read_u32_le();
    auto color_data = tmp_stream.read(color_size);
    decompress_sgd_bgr(color_data, output_stream);

    return output_stream.read_to_eof();
}

static std::vector<std::unique_ptr<Region>> read_region_data(io::Stream &stream)
{
    std::vector<std::unique_ptr<Region>> regions;
    while (stream.tell() < stream.size())
    {
        stream.skip(4);
        size_t regions_size = stream.read_u32_le();
        size_t region_count = stream.read_u16_le();
        size_t meta_format = stream.read_u16_le();
        size_t bytes_left = stream.size() - stream.tell();
        if (meta_format != 4)
            throw err::NotSupportedError("Unexpected meta format");
        if (regions_size != bytes_left)
            throw err::CorruptDataError("Region size mismatch");

        for (auto i : util::range(region_count))
        {
            auto region = std::make_unique<Region>();
            region->x = stream.read_u16_le();
            region->y = stream.read_u16_le();
            region->width = stream.read_u16_le();
            region->height = stream.read_u16_le();
            regions.push_back(std::move(region));
        }

        if (stream.tell() + 4 >= stream.size())
            break;
        stream.skip(4);
    }
    return regions;
}

static pix::Grid read_pixels(
    const bstr &input,
    CompressionType compression_type,
    size_t size_original,
    size_t width,
    size_t height)
{
    if (compression_type == CompressionType::None)
        return pix::Grid(width, height, input, pix::Format::BGRA8888);

    if (compression_type == CompressionType::Sgd)
    {
        auto decompressed = decompress_sgd(input, size_original);
        return pix::Grid(width, height, decompressed, pix::Format::BGRA8888);
    }

    if (compression_type == CompressionType::Png)
    {
        fmt::png::PngImageDecoder png_decoder;
        File tmp_file;
        tmp_file.stream.write(input);
        return png_decoder.decode(tmp_file);
    }

    throw err::NotSupportedError("Unsupported compression type");
}

bool MgdImageDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid MgdImageDecoder::decode_impl(File &input_file) const
{
    input_file.stream.skip(magic.size());

    u16 data_offset = input_file.stream.read_u16_le();
    u16 format = input_file.stream.read_u16_le();
    input_file.stream.skip(4);
    u16 width = input_file.stream.read_u16_le();
    u16 height = input_file.stream.read_u16_le();
    u32 size_original = input_file.stream.read_u32_le();
    u32 size_compressed_total = input_file.stream.read_u32_le();
    const auto compression_type
        = static_cast<const CompressionType>(input_file.stream.read_u32_le());
    input_file.stream.skip(64);

    const size_t size_compressed = input_file.stream.read_u32_le();
    if (size_compressed_total != size_compressed + 4)
        throw err::CorruptDataError("Compressed data size mismatch");

    auto image = read_pixels(
        input_file.stream.read(size_compressed),
        compression_type,
        size_original,
        width,
        height);
    read_region_data(input_file.stream);
    return image;
}

static auto dummy = fmt::register_fmt<MgdImageDecoder>("nsystem/mgd");
