#include "fmt/qlie/dpng_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::qlie;

static const bstr magic = "DPNG"_b;

bool DpngConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> DpngConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    file.io.skip(4);
    size_t file_count = file.io.read_u32_le();
    size_t width = file.io.read_u32_le();
    size_t height = file.io.read_u32_le();

    pix::Grid pixels(width, height);
    for (auto i : util::range(file_count))
    {
        size_t region_x = file.io.read_u32_le();
        size_t region_y = file.io.read_u32_le();
        size_t region_width = file.io.read_u32_le();
        size_t region_height = file.io.read_u32_le();
        size_t region_data_size = file.io.read_u32_le();
        file.io.skip(8);

        if (!region_data_size)
            continue;

        auto region = util::Image::from_boxed(file.io.read(region_data_size));
        for (auto x : util::range(region_width))
        for (auto y : util::range(region_height))
            pixels.at(region_x + x, region_y + y) = region->pixels().at(x, y);
    }

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<DpngConverter>("qlie/dpng");
