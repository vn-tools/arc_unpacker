#include "dec/gs/gs_image_decoder.h"
#include "algo/pack/lzss.h"
#include "err.h"

using namespace au;
using namespace au::dec::gs;

static const bstr magic = "\x00\x00\x04\x00"_b;

bool GsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image GsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto size_comp = input_file.stream.read_le<u32>();
    auto size_orig = input_file.stream.read_le<u32>();
    auto header_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    auto width = input_file.stream.read_le<u32>();
    auto height = input_file.stream.read_le<u32>();
    auto depth = input_file.stream.read_le<u32>();
    bool use_transparency = input_file.stream.read_le<u32>() > 0;

    input_file.stream.seek(header_size);
    auto data = input_file.stream.read(size_comp);
    data = algo::pack::lzss_decompress(data, size_orig);

    if (depth == 8)
    {
        res::Palette palette(256, data, res::PixelFormat::BGRA8888);
        for (auto &c : palette)
            c.a = 0xFF;
        return res::Image(width, height, data.substr(256 * 4), palette);
    }

    if (depth == 32)
    {
        res::Image image(width, height, data, res::PixelFormat::BGRA8888);
        if (!use_transparency)
            for (auto &c : image)
                c.a = 0xFF;
        return image;
    }

    throw err::UnsupportedBitDepthError(depth);
}

static auto _ = dec::register_decoder<GsImageDecoder>("gs/gfx");
