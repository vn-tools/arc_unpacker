#include "fmt/glib/jpeg_pgx_image_decoder.h"
#include "fmt/glib/custom_lzss.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/range.h"

// This is a bit different from plain PGX - namely, it involves two LZSS passes.

using namespace au;
using namespace au::fmt::glib;

static const bstr magic = "PGX\x00"_b;

static bstr extract_pgx_stream(const bstr &jpeg_data)
{
    bstr output;
    output.reserve(jpeg_data.size());
    io::BufferedIO jpeg_io(jpeg_data);
    jpeg_io.skip(2); // soi
    jpeg_io.skip(2); // header chunk
    jpeg_io.skip(jpeg_io.read_u16_be() - 2);
    while (jpeg_io.read_u16_be() == 0xFFE3)
        output += jpeg_io.read(jpeg_io.read_u16_be() - 2);
    return output;
}

bool JpegPgxImageDecoder::is_recognized_impl(File &file) const
{
    u16 marker = file.io.read_u16_be();
    // soi
    if (marker != 0xFFD8)
        return false;
    marker = file.io.read_u16_be();
    // header chunk
    if (marker != 0xFFE0)
        return false;
    file.io.skip(file.io.read_u16_be() - 2);
    // PGX start
    marker = file.io.read_u16_be();
    if (marker != 0xFFE3)
        return false;
    if (file.io.read_u16_be() < magic.size())
        return false;
    return file.io.read(magic.size()) == magic;
}

pix::Grid JpegPgxImageDecoder::decode_impl(File &file) const
{
    auto pgx_data = extract_pgx_stream(file.io.read_to_eof());
    io::BufferedIO pgx_io(pgx_data);

    pgx_io.skip(magic.size());
    pgx_io.skip(4);
    size_t width = pgx_io.read_u32_le();
    size_t height = pgx_io.read_u32_le();
    bool transparent = pgx_io.read_u16_le();
    pgx_io.skip(2);
    size_t source_size = pgx_io.read_u32_le();
    size_t target_size = width * height * 4;
    pgx_io.skip(8);

    if (!transparent)
    {
        pgx_io.skip(8);
        auto tmp1 = pgx_io.read_u32_le();
        auto tmp2 = pgx_io.read_u32_le();
        auto extra_size = (tmp2 & 0x00FF00FF) | (tmp1 & 0xFF00FF00);
        // why?
        custom_lzss_decompress(pgx_io, extra_size);
    }

    auto target = custom_lzss_decompress(pgx_io.read_to_eof(), target_size);

    pix::Grid pixels(width, height, target, pix::Format::BGRA8888);
    if (!transparent)
        for (auto &c : pixels)
            c.a = 0xFF;

    return pixels;
}

static auto dummy = fmt::register_fmt<JpegPgxImageDecoder>("glib/jpeg-pgx");
