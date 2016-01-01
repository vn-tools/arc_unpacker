#include "dec/glib/pgx_image_decoder.h"
#include "algo/range.h"
#include "dec/glib/custom_lzss.h"

using namespace au;
using namespace au::dec::glib;

static const bstr magic = "PGX\x00"_b;

bool PgxImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PgxImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size() + 4);
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto transparent = input_file.stream.read_u16_le() != 0;
    input_file.stream.skip(2);
    const auto source_size = input_file.stream.read_u32_le();
    const auto target_size = width * height * 4;

    input_file.stream.seek(input_file.stream.size() - source_size);
    const auto source = input_file.stream.read(source_size);
    const auto target = custom_lzss_decompress(source, target_size);
    res::Image image(width, height, target, res::PixelFormat::BGRA8888);
    if (!transparent)
        for (auto &c : image)
            c.a = 0xFF;
    return image;
}

static auto _ = dec::register_decoder<PgxImageDecoder>("glib/pgx");
