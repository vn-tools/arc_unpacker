#include "fmt/glib/pgx_image_decoder.h"
#include "algo/range.h"
#include "fmt/glib/custom_lzss.h"

using namespace au;
using namespace au::fmt::glib;

static const bstr magic = "PGX\x00"_b;

bool PgxImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PgxImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    input_file.stream.skip(4);
    size_t width = input_file.stream.read_u32_le();
    size_t height = input_file.stream.read_u32_le();
    bool transparent = input_file.stream.read_u16_le();
    input_file.stream.skip(2);
    size_t source_size = input_file.stream.read_u32_le();
    size_t target_size = width * height * 4;

    input_file.stream.seek(input_file.stream.size() - source_size);
    auto source = input_file.stream.read(source_size);

    auto target = custom_lzss_decompress(source, target_size);

    res::Image image(width, height, target, res::PixelFormat::BGRA8888);
    if (!transparent)
        for (auto &c : image)
            c.a = 0xFF;

    return image;
}

static auto dummy = fmt::register_fmt<PgxImageDecoder>("glib/pgx");
