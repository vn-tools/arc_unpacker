#include "dec/kiss/custom_png_image_decoder.h"
#include <map>
#include "algo/range.h"
#include "dec/png/png_image_decoder.h"

using namespace au;
using namespace au::dec::kiss;

static const bstr magic = "\x89PNG"_b;

bool CustomPngImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image CustomPngImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto decoder = dec::png::PngImageDecoder();
    std::map<std::string, bstr> chunks;
    auto image = decoder.decode(
        logger,
        input_file,
        [&](const std::string &name, const bstr &data)
        {
            chunks[name] = data;
        });
    if (chunks.find("xPAL") != chunks.end())
    {
        res::Palette palette(256, chunks["xPAL"], res::PixelFormat::BGR888X);
        for (const auto y : algo::range(image.height()))
        for (const auto x : algo::range(image.width()))
        {
            const auto pal_color = palette.at(image.at(x, y).r);
            image.at(x, y).r = pal_color.r;
            image.at(x, y).g = pal_color.g;
            image.at(x, y).b = pal_color.b;
        }
    }
    return image;
}

static auto _ = dec::register_decoder<CustomPngImageDecoder>("kiss/custom-png");
