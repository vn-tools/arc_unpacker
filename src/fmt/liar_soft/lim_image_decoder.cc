#include "fmt/liar_soft/lim_image_decoder.h"
#include "err.h"
#include "fmt/liar_soft/cg_decompress.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const bstr magic = "LM"_b;

bool LimImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    if (!(input_file.stream.read_u16_le() & 0x10))
        return false;
    return true;
}

pix::Image LimImageDecoder::decode_impl(io::File &input_file) const
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
    pix::Image image(width, height, output, pix::Format::BGR565);

    if (!input_file.stream.eof())
    {
        output.resize(canvas_size);
        cg_decompress(output, 0, 1, input_file.stream, 1);
        for (auto &c : output)
            c ^= 0xFF;
        pix::Image mask(width, height, output, pix::Format::Gray8);
        image.apply_mask(mask);
    }

    return image;
}

static auto dummy = fmt::register_fmt<LimImageDecoder>("liar-soft/lim");
