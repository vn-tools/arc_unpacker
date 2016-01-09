#include "dec/fvp/nvsg_image_decoder.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::fvp;

static const bstr hzc1_magic = "hzc1"_b;
static const bstr nvsg_magic = "NVSG"_b;

bool NvsgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(hzc1_magic.size()) != hzc1_magic)
        return false;
    input_file.stream.skip(8);
    return input_file.stream.read(nvsg_magic.size()) == nvsg_magic;
}

res::Image NvsgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(hzc1_magic.size());
    size_t uncompressed_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4); // nvsg header size
    input_file.stream.skip(nvsg_magic.size());
    input_file.stream.skip(2);
    size_t format = input_file.stream.read_le<u16>();
    size_t width = input_file.stream.read_le<u16>();
    size_t height = input_file.stream.read_le<u16>();
    input_file.stream.skip(8);
    size_t image_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);

    bstr data = algo::pack::zlib_inflate(input_file.stream.read_to_eof());

    res::PixelFormat pixel_format;
    switch (format)
    {
        case 0:
            pixel_format = res::PixelFormat::BGR888;
            break;

        case 1:
            pixel_format = res::PixelFormat::BGRA8888;
            break;

        case 2:
            height *= image_count;
            pixel_format = res::PixelFormat::BGRA8888;
            break;

        case 3:
            pixel_format = res::PixelFormat::Gray8;
            break;

        case 4:
            for (auto i : algo::range(data.size()))
                if (data.get<u8>()[i])
                    data.get<u8>()[i] = 255;
            pixel_format = res::PixelFormat::Gray8;
            break;

        default:
            throw err::NotSupportedError("Unexpected pixel format");
    }

    return res::Image(width, height, data, pixel_format);
}

static auto _ = dec::register_decoder<NvsgImageDecoder>("fvp/nvsg");
