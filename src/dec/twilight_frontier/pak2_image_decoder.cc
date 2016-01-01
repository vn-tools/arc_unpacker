#include "dec/twilight_frontier/pak2_image_decoder.h"
#include <map>
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/virtual_file_system.h"

using namespace au;
using namespace au::dec::twilight_frontier;

bool Pak2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("cv2");
}

res::Image Pak2ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto bit_depth = input_file.stream.read_u8();
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto stride = input_file.stream.read_u32_le();
    const auto palette_number = input_file.stream.read_u32_le();
    io::MemoryStream source_stream(input_file.stream);

    std::shared_ptr<res::Palette> palette;
    if (bit_depth == 8)
    {
        const auto palette_path = input_file.path.parent()
            / algo::format("palette%03d.pal", palette_number);
        auto palette_file = util::VirtualFileSystem::get_by_path(palette_path);

        if (!palette_file)
        {
            logger.warn("Palette %s not found\n", palette_path.c_str());
            palette = std::make_shared<res::Palette>(256);
        }
        else
        {
            palette = std::make_shared<res::Palette>(
                256,
                palette_file->stream.seek(1).read_to_eof(),
                res::PixelFormat::BGRA5551);
        }
    }

    res::Image image(width, height);
    for (const size_t y : algo::range(height))
    for (const size_t x : algo::range(stride))
    {
        res::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
            case 24:
                pixel = res::read_pixel<res::PixelFormat::BGRA8888>(
                    source_stream);
                break;

            case 8:
                pixel = (*palette)[source_stream.read_u8()];
                break;

            default:
                throw err::UnsupportedBitDepthError(bit_depth);
        }

        if (x < width)
            image.at(x, y) = pixel;
    }

    return image;
}

static auto _ = dec::register_decoder<Pak2ImageDecoder>(
    "twilight-frontier/pak2-gfx");
