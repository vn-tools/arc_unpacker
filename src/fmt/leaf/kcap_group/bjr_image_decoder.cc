#include "fmt/leaf/kcap_group/bjr_image_decoder.h"
#include <boost/filesystem/path.hpp>
#include "fmt/microsoft/bmp_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "BM"_b;

bool BjrImageDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("bjr");
}

pix::Grid BjrImageDecoder::decode_impl(File &file) const
{
    fmt::microsoft::BmpImageDecoder bmp_image_decoder;
    const auto input = bmp_image_decoder.decode(file);

    const boost::filesystem::path path(file.name);
    const auto name = path.filename().string();
    u32 key1 = 0x10000;
    u32 key2 = 0;
    u32 key3 = 0;
    for (auto i : util::range(name.size()))
    {
        key1 -= name[i];
        key2 += name[i];
        key3 ^= name[i];
    }

    pix::Grid output(input.width(), input.height());
    for (auto y : util::range(output.height()))
    {
        key3 += 7;

        const auto src_line = &input.at(0, key3 % output.height());
        const auto dst_line = &output.at(0, y);
        for (auto x : util::range(output.width()))
        {
            if (x & 1)
            {
                dst_line[x].b = 0x10000 - key2 + src_line[x].b;
                dst_line[x].g = 0x100FF - key1 - src_line[x].g;
                dst_line[x].r = 0x10000 - key2 + src_line[x].r;
            }
            else
            {
                dst_line[x].b = 0x100FF - key1 - src_line[x].b;
                dst_line[x].g = 0x10000 - key2 + src_line[x].g;
                dst_line[x].r = 0x100FF - key1 - src_line[x].r;
            }
            dst_line[x].a = 0xFF;
        }
    }

    return output;
}

static auto dummy = fmt::register_fmt<BjrImageDecoder>("leaf/bjr");
