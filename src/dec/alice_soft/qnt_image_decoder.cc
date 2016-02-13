#include "dec/alice_soft/qnt_image_decoder.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic = "QNT\x00"_b;

namespace
{
    enum class Version : u32
    {
        Version0,
        Version1,
        Version2,
    };
}

static void deinterleave(res::Image &image, const bstr &input)
{
    io::MemoryStream input_stream(input);

    size_t x, y;
    for (const auto i : algo::range(3))
    {
        for (y = 0; y < image.height() - 1; y += 2)
        {
            for (x = 0; x < image.width() - 1; x += 2)
            {
                image.at(x, y)[i] = input_stream.read<u8>();
                image.at(x, y + 1)[i] = input_stream.read<u8>();
                image.at(x + 1, y)[i] = input_stream.read<u8>();
                image.at(x + 1, y + 1)[i] = input_stream.read<u8>();
            }
            if (x != image.width())
            {
                image.at(x, y)[i] = input_stream.read<u8>();
                image.at(x, y + 1)[i] = input_stream.read<u8>();
                input_stream.skip(2);
            }
        }
        if (y != image.height())
        {
            for (x = 0; x < image.width() - 1; x += 2)
            {
                image.at(x, y)[i] = input_stream.read<u8>();
                input_stream.skip(1);
                image.at(x + 1, y)[i] = input_stream.read<u8>();
                input_stream.skip(1);
            }
            if (x != image.width())
            {
                image.at(x, y)[i] = input_stream.read<u8>();
                input_stream.skip(3);
            }
        }
    }
}

static void apply_differences(res::Image &image)
{
    for (const auto x : algo::range(1, image.width()))
    for (const auto c : algo::range(3))
        image.at(x, 0)[c] = image.at(x - 1, 0)[c] - image.at(x, 0)[c];

    for (const auto y : algo::range(1, image.height()))
    for (const auto c : algo::range(3))
        image.at(0, y)[c] = image.at(0, y - 1)[c] - image.at(0, y)[c];

    for (const auto y : algo::range(1, image.height()))
    for (const auto x : algo::range(1, image.width()))
    for (const auto c : algo::range(3))
    {
        u8 ax = image.at(x - 1, y)[c];
        u8 ay = image.at(x, y - 1)[c];
        image.at(x, y)[c] = (ax + ay) / 2 - image.at(x, y)[c];
    }
}

static void apply_alpha(res::Image &image, const bstr &input)
{
    if (!input.size())
    {
        for (const auto y : algo::range(image.height()))
        for (const auto x : algo::range(image.width()))
            image.at(x, y).a = 0xFF;
        return;
    }

    io::MemoryStream input_stream(input);

    image.at(0, 0).a = input_stream.read<u8>();
    for (const auto x : algo::range(1, image.width()))
        image.at(x, 0).a = image.at(x - 1, 0).a - input_stream.read<u8>();
    if (image.width() & 1)
        input_stream.skip(1);

    for (const auto y : algo::range(1, image.height()))
    {
        image.at(0, y).a = image.at(0, y - 1).a - input_stream.read<u8>();
        for (const auto x : algo::range(1, image.width()))
        {
            u8 ax = image.at(x - 1, y).a;
            u8 ay = image.at(x, y - 1).a;
            image.at(x, y).a = (ax + ay) / 2 - input_stream.read<u8>();
        }
        if (image.width() & 1)
            input_stream.skip(1);
    }
}

bool QntImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image QntImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    Version version = input_file.stream.read_le<Version>();

    if (version != Version::Version2)
        throw err::UnsupportedVersionError(static_cast<int>(version));

    input_file.stream.skip(4 * 3);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto depth = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    const auto pixel_size = input_file.stream.read_le<u32>();
    const auto alpha_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(24);

    bstr color_data = pixel_size
        ? algo::pack::zlib_inflate(input_file.stream.read(pixel_size))
        : ""_b;
    bstr alpha_data = alpha_size
        ? algo::pack::zlib_inflate(input_file.stream.read(alpha_size))
        : ""_b;

    res::Image image(width, height);
    if (color_data.size())
        deinterleave(image, color_data);
    apply_differences(image);
    apply_alpha(image, alpha_data);
    return image;
}

static auto _ = dec::register_decoder<QntImageDecoder>("alice-soft/qnt");
