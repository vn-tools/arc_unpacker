// DPNG image
//
// Company:   -
// Engine:    QLiE
// Extension: .png
// Archives:  .pack
//
// Known games:
// - Koiken Otome

#include "fmt/qlie/dpng_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::qlie;

static const std::string magic = "DPNG"_s;

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
    std::unique_ptr<char[]> pixel_data(new char[pixels_size]());
    u32 *pixel_ptr = reinterpret_cast<u32*>(pixel_data.get());

    for (auto i : util::range(file_count))
    {
        size_t region_x = file.io.read_u32_le();
        size_t region_y = file.io.read_u32_le();
        size_t region_width = file.io.read_u32_le();
        size_t region_height = file.io.read_u32_le();
        size_t region_data_size = file.io.read_u32_le();
        file.io.skip(8);

        if (region_data_size == 0)
            continue;

        io::BufferedIO io;
        io.write_from_io(file.io, region_data_size);

        std::unique_ptr<util::Image> region = util::Image::from_boxed(io);
        for (auto x : util::range(region_width))
        {
            for (auto y : util::range(region_height))
            {
                size_t x2 = x + region_x;
                size_t y2 = y + region_y;
                pixel_ptr[x2 + y2 * image_width] = region->color_at(x, y);
            }
        }
    }

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        image_width,
        image_height,
        std::string(pixel_data.get(), pixels_size),
        util::PixelFormat::RGBA);
    return image->create_file(file.name);
}
