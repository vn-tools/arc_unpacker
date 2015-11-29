#include "fmt/leaf/kcap_group/bbm_image_decoder.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

bool BbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.name.has_extension("bbm");
}

res::Image BbmImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto total_width = input_file.stream.read_u16_le();
    const auto total_height = input_file.stream.read_u16_le();

    const auto chunk_width = input_file.stream.read_u16_le();
    const auto chunk_height = input_file.stream.read_u16_le();
    const auto chunk_count_x = input_file.stream.read_u16_le();
    const auto chunk_count_y = input_file.stream.read_u16_le();
    const auto chunk_size = input_file.stream.read_u32_le();

    res::Image image(total_width, total_height);
    for (auto chunk_y : util::range(chunk_count_y))
    for (auto chunk_x : util::range(chunk_count_x))
    {
        io::MemoryStream chunk_stream(input_file.stream.read(chunk_size));
        chunk_stream.skip(5);
        auto color_num = chunk_stream.read_u16_le();
        chunk_stream.skip(11);
        const res::Palette palette(
            color_num, chunk_stream, res::PixelFormat::BGRA8888);
        const res::Image sub_image(
            chunk_width, chunk_height, chunk_stream, palette);
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
