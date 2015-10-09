#include "fmt/gs/gs_image_decoder.h"
#include "err.h"
#include "util/pack/lzss.h"

using namespace au;
using namespace au::fmt::gs;

static const bstr magic = "\x00\x00\x04\x00"_b;

bool GsImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid GsImageDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());
    auto size_comp = file.io.read_u32_le();
    auto size_orig = file.io.read_u32_le();
    auto header_size = file.io.read_u32_le();
    file.io.skip(4);
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto depth = file.io.read_u32_le();
    bool use_transparency = file.io.read_u32_le() > 0;

    file.io.seek(header_size);
    auto data =  file.io.read(size_comp);
    data = util::pack::lzss_decompress_bytewise(data, size_orig);

    if (depth == 8)
    {
        pix::Palette palette(256, data, pix::Format::BGRA8888);
        for (auto &c : palette)
            c.a = 0xFF;
        return pix::Grid(width, height, data.substr(256 * 4), palette);
    }

    if (depth == 32)
    {
        auto pixels = pix::Grid(width, height, data, pix::Format::BGRA8888);
        if (!use_transparency)
            for (auto &c : pixels)
                c.a = 0xFF;
        return pixels;
    }

    throw err::UnsupportedBitDepthError(depth);
}

static auto dummy = fmt::Registry::add<GsImageDecoder>("gs/gfx");
