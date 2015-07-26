// NVSG image file
//
// Company:   Favorite
// Engine:    Favorite View Point
// Extension: -
// Archives:  BIN
//
// Known games:
// - Irotoridori no Sekai

#include "fmt/fvp/nvsg_converter.h"
#include "util/zlib.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::fvp;

static const std::string hzc1_magic = "hzc1"_s;
static const std::string nvsg_magic = "NVSG"_s;

bool NvsgConverter::is_recognized_internal(File &file) const
{
    if (file.io.read(hzc1_magic.size()) != hzc1_magic)
        return false;
    file.io.skip(8);
    return file.io.read(nvsg_magic.size()) == nvsg_magic;
}

std::unique_ptr<File> NvsgConverter::decode_internal(File &file) const
{
    file.io.skip(hzc1_magic.size());
    size_t uncompressed_size = file.io.read_u32_le();
    file.io.skip(4); // nvsg header size
    file.io.skip(nvsg_magic.size());
    file.io.skip(2);
    size_t format = file.io.read_u16_le();
    size_t width = file.io.read_u16_le();
    size_t height = file.io.read_u16_le();
    file.io.skip(8);
    size_t image_count = file.io.read_u32_le();
    file.io.skip(8);

    std::string data = util::zlib_inflate(file.io.read_until_end());
    if (data.size() != uncompressed_size)
        throw std::runtime_error("Unexpected data size");

    util::PixelFormat pixel_format;
    switch (format)
    {
        case 0:
            if (width * height * 3 != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            pixel_format = util::PixelFormat::BGR;
            break;

        case 1:
            if (width * height * 4 != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            pixel_format = util::PixelFormat::BGRA;
            break;

        case 2:
            if (width * height * 4 * image_count != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            height *= image_count;
            pixel_format = util::PixelFormat::BGRA;
            break;

        case 3:
            if (width * height != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            pixel_format = util::PixelFormat::Grayscale;
            break;

        case 4:
        {
            u8 *ptr = reinterpret_cast<u8*>(&data[0]);
            u8 *guardian = ptr + data.size();
            while (ptr < guardian)
            {
                if (*ptr)
                    *ptr = 255;
                ptr++;
            }
            pixel_format = util::PixelFormat::Grayscale;
            break;
        }

        default:
            throw std::runtime_error("Unexpected pixel format");
    }

    auto image = util::Image::from_pixels(width, height, data, pixel_format);
    return image->create_file(file.name);
}
