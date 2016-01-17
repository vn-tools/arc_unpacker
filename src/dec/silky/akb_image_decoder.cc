#include "dec/silky/akb_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::silky;

static const bstr magic = "AKB\x20"_b;

bool AkbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image AkbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto channels = input_file.stream.read_be<u32>() & 0x40 ? 3 : 4;
    const auto background
        = res::read_pixel<res::PixelFormat::BGRA8888>(input_file.stream);
    const auto x1 = input_file.stream.read_le<s32>();
    const auto y1 = input_file.stream.read_le<s32>();
    const auto x2 = input_file.stream.read_le<s32>();
    const auto y2 = input_file.stream.read_le<s32>();
    if (y2 <= y1 || x2 <= x1)
        throw err::BadDataSizeError();

    const auto canvas_width = x2 - x1;
    const auto canvas_height = y2 - y1;
    const auto canvas_stride = canvas_width * channels;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), canvas_stride * canvas_height);

    for (const auto y1 : algo::range(canvas_height / 2))
    {
        const auto y2 = canvas_height - 1 - y1;
        auto source_ptr = &data[y1 * canvas_stride];
        auto target_ptr = &data[y2 * canvas_stride];
        for (const auto x : algo::range(canvas_stride))
            std::swap(source_ptr[x], target_ptr[x]);
    }

    for (const auto x : algo::range(channels, canvas_stride))
        data[x] += data[x - channels];
    for (const auto y : algo::range(1, canvas_height))
    {
        auto source_ptr = &data[(y - 1) * canvas_stride];
        auto target_ptr = &data[y * canvas_stride];
        for (const auto x : algo::range(canvas_stride))
            target_ptr[x] += source_ptr[x];
    }

    const auto fmt = channels == 4
        ? res::PixelFormat::BGRA8888
        : res::PixelFormat::BGR888;
    res::Image overlay(x2 - x1, y2 - y1, data, fmt);

    auto image = res::Image(width, height);
    for (auto &c : image)
        c = background;
    image.overlay(overlay, x1, y1, res::Image::OverlayKind::OverwriteAll);
    return image;
}

static auto _ = dec::register_decoder<AkbImageDecoder>("silky/akb");
