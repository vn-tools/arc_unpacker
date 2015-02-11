// MGD image
//
// Company:   -
// Engine:    -
// Extension: .MGD
// Archives:  FJSYS

#include <cassert>
#include "buffered_io.h"
#include "endian.h"
#include "formats/gfx/mgd_converter.h"
#include "formats/image.h"
#include "io.h"

namespace
{
    const std::string magic("MGD ", 4);

    typedef enum
    {
        COMPRESSION_NONE = 0,
        COMPRESSION_SGD = 1,
        COMPRESSION_PNG = 2,
    } CompressionType;

    typedef struct
    {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
    } Region;

    void decompress_sgd_alpha(
        const uint8_t *&input_ptr,
        const uint8_t *const input_guardian,
        uint8_t *&output_ptr,
        const uint8_t *const output_guardian)
    {
        output_ptr += 3; //ignore first RGB
        while (input_ptr < input_guardian)
        {
            uint16_t flag = le16toh(*(uint16_t*)input_ptr);
            input_ptr += 2;
            if (flag & 0x8000)
            {
                size_t pixels = (flag & 0x7fff) + 1;
                uint8_t alpha = *input_ptr ++;
                size_t i;
                for (i = 0; i < pixels; i ++)
                {
                    if (output_ptr > output_guardian)
                    {
                        throw std::runtime_error(
                            "Trying to write alpha beyond EOF");
                    }
                    *output_ptr = alpha ^ 0xff;
                    output_ptr += 4;
                }
            }

            else
            {
                while (flag -- && input_ptr < input_guardian)
                {
                    uint8_t alpha = *input_ptr;
                    input_ptr ++;
                    if (output_ptr > output_guardian)
                    {
                        throw std::runtime_error(
                            "Trying to write alpha beyond EOF");
                    }
                    *output_ptr = alpha ^ 0xff;
                    output_ptr += 4;
                }
            }
        }
    }

    void decompress_sgd_bgr_strategy_1(
        const uint8_t *&input_ptr,
        const uint8_t *const input_guardian,
        uint8_t *&output_ptr,
        const uint8_t *const output_guardian,
        uint8_t flag)
    {
        size_t pixels = flag & 0x3f;
        uint8_t b = output_ptr[-4];
        uint8_t g = output_ptr[-3];
        uint8_t r = output_ptr[-2];
        for (size_t i = 0; i < pixels; i ++)
        {
            if (input_ptr + 2 > input_guardian)
                throw std::runtime_error("Trying to read length beyond EOF");

            uint16_t delta = le16toh(*(uint16_t*)input_ptr);
            input_ptr += 2;

            if (delta & 0x8000)
            {
                b += delta & 0x1f;
                g += (delta >> 5) & 0x1f;
                r += (delta >> 10) & 0x1f;
            }
            else
            {
                b += ( delta        & 0xf) * (delta &   0x10 ? -1 : 1);
                g += ((delta >>  5) & 0xf) * (delta &  0x200 ? -1 : 1);
                r += ((delta >> 10) & 0xf) * (delta & 0x4000 ? -1 : 1);
            }

            if (output_ptr + 4 > output_guardian)
                throw std::runtime_error("Trying to write colors beyond EOF");

            *output_ptr ++ = b;
            *output_ptr ++ = g;
            *output_ptr ++ = r;
            output_ptr ++;
        }
    }

    void decompress_sgd_bgr_strategy_2(
        const uint8_t *&input_ptr,
        const uint8_t *const input_guardian,
        uint8_t *&output_ptr,
        const uint8_t *const output_guardian,
        uint8_t flag)
    {
        if (input_ptr + 3 > input_guardian)
            throw std::runtime_error("Trying to read colors beyond EOF");

        size_t pixels = (flag & 0x3f) + 1;
        uint8_t b = *input_ptr ++;
        uint8_t g = *input_ptr ++;
        uint8_t r = *input_ptr ++;
        for (size_t i = 0; i < pixels; i ++)
        {
            if (output_ptr + 4 > output_guardian)
                throw std::runtime_error("Trying to write colors beyond EOF");

            *output_ptr ++ = b;
            *output_ptr ++ = g;
            *output_ptr ++ = r;
            output_ptr ++;
        }
    }

    void decompress_sgd_bgr_strategy_3(
        const uint8_t *&input_ptr,
        const uint8_t *const input_guardian,
        uint8_t *&output_ptr,
        const uint8_t *const output_guardian,
        uint8_t flag)
    {
        size_t pixels = flag;
        for (size_t i = 0; i < pixels; i ++)
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

            *output_ptr ++ = *input_ptr ++;
            *output_ptr ++ = *input_ptr ++;
            *output_ptr ++ = *input_ptr ++;
            output_ptr ++;
        }
    }

    void decompress_sgd_bgr(
        const uint8_t *&input_ptr,
        const uint8_t *const input_guardian,
        uint8_t *&output_ptr,
        const uint8_t *const output_guardian)
    {
        while (input_ptr < input_guardian)
        {
            uint8_t flag = *input_ptr ++;
            switch (flag & 0xc0)
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

    void decompress_sgd(
        const uint8_t *const input,
        size_t input_size,
        uint8_t *const output,
        size_t output_size)
    {
        assert(input != nullptr);
        assert(output != nullptr);

        size_t length;
        const uint8_t *input_guardian;
        const uint8_t *output_guardian = output + output_size;
        uint8_t *output_ptr = output;

        const uint8_t *input_ptr = input;
        length = le32toh(*reinterpret_cast<const int32_t*>(input_ptr));
        input_ptr += 4;
        input_guardian = input_ptr + length;
        if (length > input_size)
            throw std::runtime_error("Insufficient alpha channel data");

        decompress_sgd_alpha(
            input_ptr,
            input_guardian,
            output_ptr,
            output_guardian);

        length = le32toh(*reinterpret_cast<const uint32_t*>(input_ptr));
        input_ptr += 4;
        input_guardian = (const uint8_t*)input_ptr + length;
        if (length > input_size)
            throw std::runtime_error("Insufficient color data");

        output_ptr = output;
        decompress_sgd_bgr(
            input_ptr,
            input_guardian,
            output_ptr,
            output_guardian);
    }

    std::vector<std::unique_ptr<Region>> read_region_data(IO &file_io)
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

            for (size_t i = 0; i < region_count; i ++)
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

    std::unique_ptr<Image> read_image(
        IO &file_io,
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
                return Image::from_pixels(
                    image_width,
                    image_height,
                    data_compressed,
                    IMAGE_PIXEL_FORMAT_BGRA);

            case COMPRESSION_SGD:
            {
                std::unique_ptr<char> data_uncompressed(
                    new char[size_original]);

                decompress_sgd(
                    reinterpret_cast<const uint8_t*>(data_compressed.data()),
                    size_compressed,
                    reinterpret_cast<uint8_t*>(data_uncompressed.get()),
                    size_original);

                return Image::from_pixels(
                    image_width,
                    image_height,
                    std::string(data_uncompressed.get(), size_original),
                    IMAGE_PIXEL_FORMAT_BGRA);
            }

            case COMPRESSION_PNG:
            {
                BufferedIO buffered_io(data_compressed);
                return Image::from_boxed(buffered_io);
            }

            default:
                throw std::runtime_error("Unsupported compression type");
        }
    }
}

void MgdConverter::decode_internal(VirtualFile &file) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a MGD graphic file");

    uint16_t data_offset = file.io.read_u16_le();
    uint16_t format = file.io.read_u16_le();
    file.io.skip(4);
    uint16_t image_width = file.io.read_u16_le();
    uint16_t image_height = file.io.read_u16_le();
    uint32_t size_original = file.io.read_u32_le();
    uint32_t size_compressed_total = file.io.read_u32_le();
    CompressionType compression_type = (CompressionType)file.io.read_u32_le();
    file.io.skip(64);

    size_t size_compressed = file.io.read_u32_le();
    if (size_compressed + 4 != size_compressed_total)
        throw std::runtime_error("Compressed data size mismatch");

    std::unique_ptr<Image> image = read_image(
        file.io,
        compression_type,
        size_compressed,
        size_original,
        image_width,
        image_height);

    read_region_data(file.io);

    image->update_file(file);
}
