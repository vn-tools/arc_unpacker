#include "fmt/liar_soft/wcg_image_decoder.h"
#include "err.h"
#include "fmt/liar_soft/cg_decompress.h"
#include "io/memory_stream.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const bstr magic = "WG"_b;

bool WcgImageDecoder::is_recognized_impl(File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;

    const int version = input_file.stream.read_u16_le();
    if (((version & 0xF) != 1) || ((version & 0x1C0) != 64))
        return false;

    return true;
}

pix::Grid WcgImageDecoder::decode_impl(File &input_file) const
{
    input_file.stream.seek(magic.size());

    input_file.stream.skip(2);
    const auto depth = input_file.stream.read_u16_le();
    if (depth != 32)
        throw err::UnsupportedBitDepthError(depth);
    input_file.stream.skip(2);

    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto canvas_size = width * height;

    bstr output(canvas_size * 4);
    cg_decompress(output, 2, 4, input_file.stream, 2);
    cg_decompress(output, 0, 4, input_file.stream, 2);

    for (auto i : util::range(0, output.size(), 4))
        output[i + 3] ^= 0xFF;

    return pix::Grid(width, height, output, pix::Format::BGRA8888);
}

static auto dummy = fmt::register_fmt<WcgImageDecoder>("liar-soft/wcg");
