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
#include "util/pack/zlib.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fvp;

static const bstr hzc1_magic = "hzc1"_b;
static const bstr nvsg_magic = "NVSG"_b;

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

    bstr data = util::pack::zlib_inflate(file.io.read_to_eof());
    if (data.size() != uncompressed_size)
        throw std::runtime_error("Unexpected data size");

    pix::Format pixel_format;
    switch (format)
    {
        case 0:
            if (width * height * 3 != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            pixel_format = pix::Format::BGR888;
            break;

        case 1:
            if (width * height * 4 != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            pixel_format = pix::Format::BGRA8888;
            break;

        case 2:
            if (width * height * 4 * image_count != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            height *= image_count;
            pixel_format = pix::Format::BGRA8888;
            break;

        case 3:
            if (width * height != uncompressed_size)
                throw std::runtime_error("Unexpected data size");
            pixel_format = pix::Format::Gray8;
            break;

        case 4:
            for (auto i : util::range(data.size()))
                if (data.get<u8>()[i])
                    data.get<u8>()[i] = 255;
            pixel_format = pix::Format::Gray8;
            break;

        default:
            throw std::runtime_error("Unexpected pixel format");
    }

    pix::Grid pixels(width, height, data, pixel_format);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<NvsgConverter>("fvp/nvsg");
