#include "fmt/leaf/bbm_image_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

bool BbmImageDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("bbm");
}

pix::Grid BbmImageDecoder::decode_impl(File &file) const
{
    file.io.seek(0);
    const auto total_width = file.io.read_u16_le();
    const auto total_height = file.io.read_u16_le();

    const auto chunk_width = file.io.read_u16_le();
    const auto chunk_height = file.io.read_u16_le();
    const auto chunk_count_x = file.io.read_u16_le();
    const auto chunk_count_y = file.io.read_u16_le();
    const auto chunk_size = file.io.read_u32_le();

    pix::Grid image(total_width, total_height);
    for (auto chunk_y : util::range(chunk_count_y))
    for (auto chunk_x : util::range(chunk_count_x))
    {
        io::BufferedIO chunk_io(file.io.read(chunk_size));
        chunk_io.skip(5);
        auto color_num = chunk_io.read_u16_le();
        chunk_io.skip(11);
        const pix::Palette palette(color_num, chunk_io, pix::Format::BGRA8888);
        const pix::Grid sub_image(chunk_width, chunk_height, chunk_io, palette);
        const auto base_x = chunk_x * chunk_width;
        const auto base_y = chunk_y * chunk_height;
        for (auto y : util::range(chunk_height))
        for (auto x : util::range(chunk_width))
        {
            image.at(base_x + x, base_y + y) = sub_image.at(x, y);
        }
    }
    return image;
}

static auto dummy = fmt::register_fmt<BbmImageDecoder>("leaf/bbm");
