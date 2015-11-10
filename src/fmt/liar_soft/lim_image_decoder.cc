#include "fmt/liar_soft/lim_image_decoder.h"
#include "err.h"
#include "fmt/liar_soft/cg_decompress.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const bstr magic = "LM"_b;

bool LimImageDecoder::is_recognized_impl(File &file) const
{
    if (file.io.read(magic.size()) != magic)
        return false;
    if (!(file.io.read_u16_le() & 0x10))
        return false;
    return true;
}

pix::Grid LimImageDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());
    const auto version = file.io.read_u16_le();
    const auto depth = file.io.read_u16_le();
    const auto bits_per_channel = file.io.read_u16_le();
    if (bits_per_channel != 8)
        throw err::UnsupportedBitDepthError(bits_per_channel);

    const auto width = file.io.read_u32_le();
    const auto height = file.io.read_u32_le();
    const auto canvas_size = width * height;

    if (!(version & 0xF))
        throw err::UnsupportedVersionError(version);
    if (depth != 16)
        throw err::UnsupportedBitDepthError(depth);

    bstr output(canvas_size * 2);
    cg_decompress(output, 0, 2, file.io, 2);
    pix::Grid pixels(width, height, output, pix::Format::BGR565);

    if (!file.io.eof())
    {
        output.resize(canvas_size);
        cg_decompress(output, 0, 1, file.io, 1);
        for (auto &c : output)
            c ^= 0xFF;
        pix::Grid mask(width, height, output, pix::Format::Gray8);
        pixels.apply_mask(mask);
    }

    return pixels;
}

static auto dummy = fmt::register_fmt<LimImageDecoder>("liar-soft/lim");
