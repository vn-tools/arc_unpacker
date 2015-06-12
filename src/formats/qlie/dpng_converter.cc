// DPNG image
//
// Company:   -
// Engine:    QLiE
// Extension: .png
// Archives:  .pack
//
// Known games:
// - Koiken Otome

#include "formats/qlie/dpng_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
using namespace Formats::QLiE;

namespace
{
    const std::string magic("DPNG", 4);
}

bool DpngConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> DpngConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    file.io.skip(4);
    size_t file_count = file.io.read_u32_le();
    size_t image_width = file.io.read_u32_le();
    size_t image_height = file.io.read_u32_le();

    size_t pixels_size = image_width * image_height * 4;
    std::unique_ptr<char[]> pixel_data(new char[pixels_size]);
    u32 *pixel_ptr = reinterpret_cast<u32*>(pixel_data.get());
    for (size_t i = 0; i < image_width * image_height; i++)
        pixel_ptr[i] = 0;

    for (size_t i = 0; i < file_count; i++)
    {
        size_t region_x = file.io.read_u32_le();
        size_t region_y = file.io.read_u32_le();
        size_t region_width = file.io.read_u32_le();
        size_t region_height = file.io.read_u32_le();
        size_t region_data_size = file.io.read_u32_le();
        file.io.skip(8);

        if (region_data_size == 0)
            continue;

        BufferedIO io;
        io.write_from_io(file.io, region_data_size);

        std::unique_ptr<Image> region = Image::from_boxed(io);
        for (size_t x = 0; x < region_width; x++)
        {
            for (size_t y = 0; y < region_height; y++)
            {
                size_t x2 = x + region_x;
                size_t y2 = y + region_y;
                pixel_ptr[x2 + y2 * image_width] = region->color_at(x, y);
            }
        }
    }

    std::unique_ptr<Image> image = Image::from_pixels(
        image_width,
        image_height,
        std::string(pixel_data.get(), pixels_size),
        PixelFormat::RGBA);
    return image->create_file(file.name);
}
