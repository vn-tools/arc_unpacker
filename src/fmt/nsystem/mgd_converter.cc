// MGD image
//
// Company:   -
// Engine:    NSystem
// Extension: .MGD
// Archives:  FJSYS

#include <cassert>
#include "fmt/nsystem/mgd_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
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

    struct Region
    {
        u16 x;
        u16 y;
        u16 width;
        u16 height;
    };
}

static const bstr magic = "MGD "_b;

static void decompress_sgd_alpha(const bstr &input, io::IO &output_io)
{
    io::BufferedIO input_io(input);
    while (!input_io.eof())
    {
        auto flag = input_io.read_u16_le();
        if (flag & 0x8000)
        {
            size_t length = (flag & 0x7FFF) + 1;
            u8 alpha = input_io.read_u8();
            for (auto i : util::range(length))
            {
                output_io.skip(3);
                output_io.write_u8(alpha ^ 0xFF);
            }
        }
        else
        {
            while (flag-- && !input_io.eof())
            {
                u8 alpha = input_io.read_u8();
                output_io.skip(3);
                output_io.write_u8(alpha ^ 0xFF);
            }
        }
    }
    output_io.seek(0);
}

static void decompress_sgd_bgr_strategy_1(
    io::IO &input_io, io::IO &output_io, u8 flag)
{
    auto length = flag & 0x3F;
    output_io.skip(-4);
    u8 b = output_io.read_u8();
    u8 g = output_io.read_u8();
    u8 r = output_io.read_u8();
    output_io.skip(1);
    for (auto i : util::range(length))
    {
        u16 delta = input_io.read_u16_le();
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

        output_io.write_u8(b);
        output_io.write_u8(g);
        output_io.write_u8(r);
        output_io.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_2(
    io::IO &input_io, io::IO &output_io, u8 flag)
{
    auto length = (flag & 0x3F) + 1;
    u8 b = input_io.read_u8();
    u8 g = input_io.read_u8();
    u8 r = input_io.read_u8();
    for (auto i : util::range(length))
    {
        output_io.write_u8(b);
        output_io.write_u8(g);
        output_io.write_u8(r);
        output_io.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_3(
    io::IO &input_io, io::IO &output_io, u8 flag)
{
    auto length = flag;
    for (auto i : util::range(length))
    {
        output_io.write(input_io.read(3));
        output_io.skip(1);
    }
}

static void decompress_sgd_bgr(const bstr &input, io::IO &output_io)
{
    io::BufferedIO input_io(input);
    while (!input_io.eof())
    {
        u8 flag = input_io.read_u8();
        switch (flag & 0xC0)
        {
            case 0x80:
                decompress_sgd_bgr_strategy_1(input_io, output_io, flag);
                break;

            case 0x40:
                decompress_sgd_bgr_strategy_2(input_io, output_io, flag);
                break;

            case 0:
                decompress_sgd_bgr_strategy_3(input_io, output_io, flag);
                break;

            default:
                throw std::runtime_error("Bad decompression flag");
        }
    }
    output_io.seek(0);
}

static bstr decompress_sgd(const bstr &input, size_t output_size)
{
    bstr output;
    output.resize(output_size);
    io::BufferedIO output_io(output);

    io::BufferedIO tmp_io(input);

    auto alpha_size = tmp_io.read_u32_le();
    auto alpha_data = tmp_io.read(alpha_size);
    decompress_sgd_alpha(alpha_data, output_io);

    auto color_size = tmp_io.read_u32_le();
    auto color_data = tmp_io.read(color_size);
    decompress_sgd_bgr(color_data, output_io);

    return output_io.read_until_end();
}

static std::vector<std::unique_ptr<Region>> read_region_data(io::IO &file_io)
{
    std::vector<std::unique_ptr<Region>> regions;
    while (file_io.tell() < file_io.size())
    {
        file_io.skip(4);
        size_t regions_size = file_io.read_u32_le();
        size_t region_count = file_io.read_u16_le();
        size_t meta_format = file_io.read_u16_le();
        size_t bytes_left = file_io.size() - file_io.tell();
        if (meta_format != 4)
            throw std::runtime_error("Unexpected region format");
        if (regions_size != bytes_left)
            throw std::runtime_error("Region size mismatch");

        for (auto i : util::range(region_count))
        {
            std::unique_ptr<Region> region(new Region);
            region->x = file_io.read_u16_le();
            region->y = file_io.read_u16_le();
            region->width = file_io.read_u16_le();
            region->height = file_io.read_u16_le();
            regions.push_back(std::move(region));
        }

        if (file_io.tell() + 4 >= file_io.size())
            break;
        file_io.skip(4);
    }
    return regions;
}

static std::unique_ptr<util::Image> read_image(
    io::IO &file_io,
    CompressionType compression_type,
    size_t size_compressed,
    size_t size_original,
    size_t width,
    size_t height)
{
    bstr data_compressed = file_io.read(size_compressed);
    switch (compression_type)
    {
        case CompressionType::None:
            return util::Image::from_pixels(
                width,
                height,
                data_compressed,
                util::PixelFormat::BGRA);

        case CompressionType::Sgd:
            return util::Image::from_pixels(
                width,
                height,
                decompress_sgd(data_compressed, size_original),
                util::PixelFormat::BGRA);

        case CompressionType::Png:
            return util::Image::from_boxed(data_compressed);

        default:
            throw std::runtime_error("Unsupported compression type");
    }
}

bool MgdConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> MgdConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    u16 data_offset = file.io.read_u16_le();
    u16 format = file.io.read_u16_le();
    file.io.skip(4);
    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();
    u32 size_original = file.io.read_u32_le();
    u32 size_compressed_total = file.io.read_u32_le();
    auto compression_type = static_cast<CompressionType>(file.io.read_u32_le());
    file.io.skip(64);

    size_t size_compressed = file.io.read_u32_le();
    if (size_compressed + 4 != size_compressed_total)
        throw std::runtime_error("Compressed data size mismatch");

    std::unique_ptr<util::Image> image = read_image(
        file.io,
        compression_type,
        size_compressed,
        size_original,
        width,
        height);

    read_region_data(file.io);
    return image->create_file(file.name);
}
