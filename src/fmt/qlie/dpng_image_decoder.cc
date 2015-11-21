#include "fmt/qlie/dpng_image_decoder.h"
#include "fmt/png/png_image_decoder.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::qlie;

static const bstr magic = "DPNG"_b;

bool DpngImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid DpngImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());

    file.stream.skip(4);
    size_t file_count = file.stream.read_u32_le();
    size_t width = file.stream.read_u32_le();
    size_t height = file.stream.read_u32_le();

    fmt::png::PngImageDecoder png_image_decoder;

    pix::Grid pixels(width, height);
    for (auto i : util::range(file_count))
    {
        size_t region_x = file.stream.read_u32_le();
        size_t region_y = file.stream.read_u32_le();
        size_t region_width = file.stream.read_u32_le();
        size_t region_height = file.stream.read_u32_le();
        size_t region_data_size = file.stream.read_u32_le();
        file.stream.skip(8);

        if (!region_data_size)
            continue;

        File tmp_file;
        tmp_file.stream.write(file.stream.read(region_data_size));
        auto region = png_image_decoder.decode(tmp_file);

        for (auto x : util::range(region_width))
        for (auto y : util::range(region_height))
            pixels.at(region_x + x, region_y + y) = region.at(x, y);
    }

    return pixels;
}

static auto dummy = fmt::register_fmt<DpngImageDecoder>("qlie/dpng");
