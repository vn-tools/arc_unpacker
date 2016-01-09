#include "dec/lilim/dbm_image_decoder.h"
#include "algo/format.h"
#include "dec/lilim/common.h"
#include "err.h"

using namespace au;
using namespace au::dec::lilim;

static const bstr magic = "DM"_b;

bool DbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    input_file.stream.skip(2);
    return input_file.stream.read_le<u32>() == input_file.stream.size();
}

res::Image DbmImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 8);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto format = input_file.stream.read_le<u32>();
    if (input_file.stream.read_le<u16>() != 1)
        throw err::CorruptDataError("Expected '1'");
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto data = sysd_decompress(input_file.stream.read(size_comp));

    if (format == 1 || format == 2 || format == 3)
    {
        auto image = res::Image(width, height, data, res::PixelFormat::BGR888);
        image.flip_vertically();
        return image;
    }
    else if (format == 4)
    {
        res::Image mask(width, height / 2, data, res::PixelFormat::BGR888);
        res::Image image(
            width,
            height / 2,
            data.substr(3 * width * height / 2),
            res::PixelFormat::BGR888);
        image.apply_mask(mask);
        image.flip_vertically();
        return image;
    }

    throw err::NotSupportedError(
        algo::format("Pixel format %d is not supported", format));
}

static auto _ = dec::register_decoder<DbmImageDecoder>("lilim/dbm");
