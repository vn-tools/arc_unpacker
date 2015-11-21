#include "fmt/glib/pgx_image_decoder.h"
#include "fmt/glib/custom_lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::glib;

static const bstr magic = "PGX\x00"_b;

bool PgxImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid PgxImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());

    file.stream.skip(4);
    size_t width = file.stream.read_u32_le();
    size_t height = file.stream.read_u32_le();
    bool transparent = file.stream.read_u16_le();
    file.stream.skip(2);
    size_t source_size = file.stream.read_u32_le();
    size_t target_size = width * height * 4;

    file.stream.seek(file.stream.size() - source_size);
    auto source = file.stream.read(source_size);

    auto target = custom_lzss_decompress(source, target_size);

    pix::Grid pixels(width, height, target, pix::Format::BGRA8888);
    if (!transparent)
        for (auto &c : pixels)
            c.a = 0xFF;

    return pixels;
}

static auto dummy = fmt::register_fmt<PgxImageDecoder>("glib/pgx");
