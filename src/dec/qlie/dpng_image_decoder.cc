#include "dec/qlie/dpng_image_decoder.h"
#include "algo/range.h"
#include "dec/png/png_image_decoder.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::qlie;

static const bstr magic = "DPNG"_b;

bool DpngImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image DpngImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    const auto file_count = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();

    const auto png_image_decoder = dec::png::PngImageDecoder();

    res::Image image(width, height);
    for (const auto i : algo::range(file_count))
    {
        const auto subimage_x = input_file.stream.read_le<u32>();
        const auto subimage_y = input_file.stream.read_le<u32>();
        const auto subimage_width = input_file.stream.read_le<u32>();
        const auto subimage_height = input_file.stream.read_le<u32>();
        const auto subimage_data_size = input_file.stream.read_le<u32>();
        input_file.stream.skip(8);

        if (!subimage_data_size)
            continue;

        io::File tmp_file;
        tmp_file.stream.write(input_file.stream.read(subimage_data_size));
        const auto subimage = png_image_decoder.decode(logger, tmp_file);
        image.overlay(
            subimage,
            subimage_x,
            subimage_y,
            res::Image::OverlayKind::OverwriteAll);
    }

    return image;
}

static auto _ = dec::register_decoder<DpngImageDecoder>("qlie/dpng");
