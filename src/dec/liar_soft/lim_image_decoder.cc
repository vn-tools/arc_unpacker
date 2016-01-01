#include "dec/liar_soft/lim_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/liar_soft/cg_decompress.h"
#include "err.h"

using namespace au;
using namespace au::dec::liar_soft;

static const bstr magic = "LM"_b;

bool LimImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    if (!(input_file.stream.read_u16_le() & 0x10))
        return false;
    return true;
}

res::Image LimImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version = input_file.stream.read_u16_le();
    const auto depth = input_file.stream.read_u16_le();
    const auto bits_per_channel = input_file.stream.read_u16_le();
    if (bits_per_channel != 8)
        throw err::UnsupportedBitDepthError(bits_per_channel);

    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto canvas_size = width * height;

    if (!(version & 0xF))
        throw err::UnsupportedVersionError(version);
    if (depth != 16)
        throw err::UnsupportedBitDepthError(depth);

    bstr output(canvas_size * 2);
    cg_decompress(output, 0, 2, input_file.stream, 2);
    res::Image image(width, height, output, res::PixelFormat::BGR565);

    if (!input_file.stream.eof())
    {
        output.resize(canvas_size);
        cg_decompress(output, 0, 1, input_file.stream, 1);
        for (auto &c : output)
            c ^= 0xFF;
        res::Image mask(width, height, output, res::PixelFormat::Gray8);
        image.apply_mask(mask);
    }

    return image;
}

static auto _ = dec::register_decoder<LimImageDecoder>("liar-soft/lim");
