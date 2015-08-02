// MGD image
//
// Company:   -
// Engine:    NSystem
// Extension: .MGD
// Archives:  FJSYS

#include <cassert>
#include <stdexcept>
#include "fmt/nsystem/mgd_converter.h"
#include "io/buffered_io.h"
#include "util/endian.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nsystem;

namespace
{
    enum CompressionType
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_SGD = 1,
        COMPRESSION_PNG = 2,
    };

    struct Region
    {
        u16 x;
        u16 y;
        u16 width;
        u16 height;
    };
}

static const std::string magic = "MGD "_s;

static void decompress_sgd_alpha(
    const u8 *&input_ptr,
    const u8 *const input_guardian,
    u8 *&output_ptr,
    const u8 *const output_guardian)
{
    output_ptr += 3; //ignore first RGB
    while (input_ptr < input_guardian)
    {
        u16 flag = util::from_little_endian<u16>(
            *reinterpret_cast<const u16*>(input_ptr));
        input_ptr += 2;
        if (flag & 0x8000)
        {
            size_t pixels = (flag & 0x7FFF) + 1;
            u8 alpha = *input_ptr++;
            for (auto i : util::range(pixels))
            {
                if (output_ptr > output_guardian)
                {
                    throw std::runtime_error(
                        "Trying to write alpha beyond EOF");
                }
                *output_ptr = alpha ^ 0xFF;
                output_ptr += 4;
            }
        }
        else
        {
            while (flag-- && input_ptr < input_guardian)
            {
                u8 alpha = *input_ptr;
                input_ptr++;
                if (output_ptr > output_guardian)
                {
                    throw std::runtime_error(
                        "Trying to write alpha beyond EOF");
                }
                *output_ptr = alpha ^ 0xFF;
                output_ptr += 4;
            }
        }
    }
}

static void decompress_sgd_bgr_strategy_1(
    const u8 *&input_ptr,
    const u8 *const input_guardian,
    u8 *&output_ptr,
    const u8 *const output_guardian,
    u8 flag)
{
    size_t pixels = flag & 0x3F;
    u8 b = output_ptr[-4];
    u8 g = output_ptr[-3];
    u8 r = output_ptr[-2];
    for (auto i : util::range(pixels))
    {
        if (input_ptr + 2 > input_guardian)
            throw std::runtime_error("Trying to read length beyond EOF");

        u16 delta = util::from_little_endian<u16>(*
            reinterpret_cast<const u16*>(input_ptr));
        input_ptr += 2;

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

        if (output_ptr + 4 > output_guardian)
            throw std::runtime_error("Trying to write colors beyond EOF");

        *output_ptr++ = b;
        *output_ptr++ = g;
        *output_ptr++ = r;
        output_ptr++;
    }
}

static void decompress_sgd_bgr_strategy_2(
    const u8 *&input_ptr,
    const u8 *const input_guardian,
    u8 *&output_ptr,
    const u8 *const output_guardian,
    u8 flag)
{
    if (input_ptr + 3 > input_guardian)
        throw std::runtime_error("Trying to read colors beyond EOF");

    size_t pixels = (flag & 0x3F) + 1;
    u8 b = *input_ptr++;
    u8 g = *input_ptr++;
    u8 r = *input_ptr++;
    for (auto i : util::range(pixels))
    {
        if (output_ptr + 4 > output_guardian)
            throw std::runtime_error("Trying to write colors beyond EOF");

        *output_ptr++ = b;
        *output_ptr++ = g;
        *output_ptr++ = r;
        output_ptr++;
    }
}

static void decompress_sgd_bgr_strategy_3(
    const u8 *&input_ptr,
    const u8 *const input_guardian,
    u8 *&output_ptr,
    const u8 *const output_guardian,
    u8 flag)
{
    size_t pixels = flag;
    for (auto i : util::range(pixels))
    {
        if (input_ptr + 3 > input_guardian)
        {
            throw std::runtime_error(
                "Trying to read colors beyond EOF");
        }
        if (output_ptr + 4 > output_guardian)
        {
            throw std::runtime_error(
                "Trying to write colors beyond EOF");
        }

        *output_ptr++ = *input_ptr++;
        *output_ptr++ = *input_ptr++;
        *output_ptr++ = *input_ptr++;
        output_ptr++;
    }
}

static void decompress_sgd_bgr(
    const u8 *&input_ptr,
    const u8 *const input_guardian,
    u8 *&output_ptr,
    const u8 *const output_guardian)
{
    while (input_ptr < input_guardian)
    {
        u8 flag = *input_ptr++;
        switch (flag & 0xC0)
        {
            case 0x80:
                decompress_sgd_bgr_strategy_1(
                    input_ptr, input_guardian,
                    output_ptr, output_guardian,
                    flag);
                break;

            case 0x40:
                decompress_sgd_bgr_strategy_2(
                    input_ptr, input_guardian,
                    output_ptr, output_guardian,
                    flag);
                break;

            case 0:
                decompress_sgd_bgr_strategy_3(
                    input_ptr, input_guardian,
                    output_ptr, output_guardian,
                    flag);
                break;

            default:
                throw std::runtime_error("Bad decompression flag");
        }
    }
}

static void decompress_sgd(
    const u8 *const input,
    size_t input_size,
    u8 *const output,
    size_t output_size)
{
    assert(input != nullptr);
    assert(output != nullptr);

    size_t length;
    const u8 *input_guardian;
    const u8 *output_guardian = output + output_size;
    u8 *output_ptr = output;

    const u8 *input_ptr = input;
    length = util::from_little_endian<u32>(
        *reinterpret_cast<const u32*>(input_ptr));
    input_ptr += 4;
    input_guardian = input_ptr + length;
    if (length > input_size)
        throw std::runtime_error("Insufficient alpha channel data");

    decompress_sgd_alpha(
        input_ptr,
        input_guardian,
        output_ptr,
        output_guardian);

    length = util::from_little_endian<u32>(
        *reinterpret_cast<const u32*>(input_ptr));
    input_ptr += 4;
    input_guardian = input_ptr + length;
    if (length > input_size)
        throw std::runtime_error("Insufficient color data");

    output_ptr = output;
    decompress_sgd_bgr(
        input_ptr,
        input_guardian,
        output_ptr,
        output_guardian);
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
    size_t image_width,
    size_t image_height)
{
    std::string data_compressed = file_io.read(size_compressed);
    switch (compression_type)
    {
        case COMPRESSION_NONE:
            return util::Image::from_pixels(
                image_width,
                image_height,
                data_compressed,
                util::PixelFormat::BGRA);

        case COMPRESSION_SGD:
        {
            std::unique_ptr<char[]> data_uncompressed(
                new char[size_original]);

            decompress_sgd(
                reinterpret_cast<const u8*>(data_compressed.data()),
                size_compressed,
                reinterpret_cast<u8*>(data_uncompressed.get()),
                size_original);

            return util::Image::from_pixels(
                image_width,
                image_height,
                std::string(data_uncompressed.get(), size_original),
                util::PixelFormat::BGRA);
        }

        case COMPRESSION_PNG:
        {
            io::BufferedIO buffered_io(data_compressed);
            return util::Image::from_boxed(buffered_io);
        }

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
    u16 image_width = file.io.read_u16_le();
    u16 image_height = file.io.read_u16_le();
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
        image_width,
        image_height);

    read_region_data(file.io);

    return image->create_file(file.name);
}
