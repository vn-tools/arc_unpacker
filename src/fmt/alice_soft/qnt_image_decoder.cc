#include "fmt/alice_soft/qnt_image_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "QNT\x00"_b;

namespace
{
    enum Version
    {
        Version0,
        Version1,
        Version2,
    };
}

static void deinterleave(pix::Grid &pixels, const bstr &input)
{
    io::MemoryStream input_stream(input);

    size_t x, y;
    for (auto i : util::range(3))
    {
        for (y = 0; y < pixels.height() - 1; y += 2)
        {
            for (x = 0; x < pixels.width() - 1; x += 2)
            {
                pixels.at(x, y)[i] = input_stream.read_u8();
                pixels.at(x, y + 1)[i] = input_stream.read_u8();
                pixels.at(x + 1, y)[i] = input_stream.read_u8();
                pixels.at(x + 1, y + 1)[i] = input_stream.read_u8();
            }
            if (x != pixels.width())
            {
                pixels.at(x, y)[i] = input_stream.read_u8();
                pixels.at(x, y + 1)[i] = input_stream.read_u8();
                input_stream.skip(2);
            }
        }
        if (y != pixels.height())
        {
            for (x = 0; x < pixels.width() - 1; x += 2)
            {
                pixels.at(x, y)[i] = input_stream.read_u8();
                input_stream.skip(1);
                pixels.at(x + 1, y)[i] = input_stream.read_u8();
                input_stream.skip(1);
            }
            if (x != pixels.width())
            {
                pixels.at(x, y)[i] = input_stream.read_u8();
                input_stream.skip(3);
            }
        }
    }
}

static void apply_differences(pix::Grid &pixels)
{
    for (auto x : util::range(1, pixels.width()))
    for (auto c : util::range(3))
        pixels.at(x, 0)[c] = pixels.at(x - 1, 0)[c] - pixels.at(x, 0)[c];

    for (auto y : util::range(1, pixels.height()))
    for (auto c : util::range(3))
        pixels.at(0, y)[c] = pixels.at(0, y - 1)[c] - pixels.at(0, y)[c];

    for (auto y : util::range(1, pixels.height()))
    for (auto x : util::range(1, pixels.width()))
    for (auto c : util::range(3))
    {
        u8 ax = pixels.at(x - 1, y)[c];
        u8 ay = pixels.at(x, y - 1)[c];
        pixels.at(x, y)[c] = (ax + ay) / 2 - pixels.at(x, y)[c];
    }
}

static void apply_alpha(pix::Grid &pixels, const bstr &input)
{
    if (!input.size())
    {
        for (auto y : util::range(pixels.height()))
        for (auto x : util::range(pixels.width()))
            pixels.at(x, y).a = 0xFF;
        return;
    }

    io::MemoryStream input_stream(input);

    pixels.at(0, 0).a = input_stream.read_u8();
    for (auto x : util::range(1, pixels.width()))
        pixels.at(x, 0).a = pixels.at(x - 1, 0).a - input_stream.read_u8();
    if (pixels.width() & 1)
        input_stream.skip(1);

    for (auto y : util::range(1, pixels.height()))
    {
        pixels.at(0, y).a = pixels.at(0, y - 1).a - input_stream.read_u8();
        for (auto x : util::range(1, pixels.width()))
        {
            u8 ax = pixels.at(x - 1, y).a;
            u8 ay = pixels.at(x, y - 1).a;
            pixels.at(x, y).a = (ax + ay) / 2 - input_stream.read_u8();
        }
        if (pixels.width() & 1)
            input_stream.skip(1);
    }
}

bool QntImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid QntImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());
    Version version = static_cast<Version>(file.stream.read_u32_le());

    if (version != Version2)
        throw err::UnsupportedVersionError(version);

    file.stream.skip(4 * 3);
    auto width = file.stream.read_u32_le();
    auto height = file.stream.read_u32_le();
    auto depth = file.stream.read_u32_le();
    file.stream.skip(4);
    auto pixel_size = file.stream.read_u32_le();
    auto alpha_size = file.stream.read_u32_le();
    file.stream.skip(24);

    bstr color_data = pixel_size
        ? util::pack::zlib_inflate(file.stream.read(pixel_size))
        : ""_b;
    bstr alpha_data = alpha_size
        ? util::pack::zlib_inflate(file.stream.read(alpha_size))
        : ""_b;

    pix::Grid pixels(width, height);
    if (color_data.size())
        deinterleave(pixels, color_data);
    apply_differences(pixels);
    apply_alpha(pixels, alpha_data);

    return pixels;
}

static auto dummy = fmt::register_fmt<QntImageDecoder>("alice-soft/qnt");
