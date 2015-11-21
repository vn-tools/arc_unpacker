#include "fmt/lilim/dbm_image_decoder.h"
#include "err.h"
#include "fmt/lilim/common.h"
#include "util/format.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "DM"_b;

bool DbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    input_file.stream.skip(2);
    return input_file.stream.read_u32_le() == input_file.stream.size();
}

pix::Grid DbmImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 8);
    const auto width = input_file.stream.read_u16_le();
    const auto height = input_file.stream.read_u16_le();
    const auto format = input_file.stream.read_u32_le();
    if (input_file.stream.read_u16_le() != 1)
        throw err::CorruptDataError("Expected '1'");
    const auto size_comp = input_file.stream.read_u32_le();
    const auto data = sysd_decompress(input_file.stream.read(size_comp));

    if (format == 1 || format == 2 || format == 3)
    {
        auto image = pix::Grid(width, height, data, pix::Format::BGR888);
        image.flip_vertically();
        return image;
    }
    else if (format == 4)
    {
        auto mask = pix::Grid(width, height / 2, data, pix::Format::BGR888);
        auto image = pix::Grid(
            width,
            height / 2,
            data.substr(3 * width * height / 2),
            pix::Format::BGR888);
        image.apply_mask(mask);
        image.flip_vertically();
        return image;
    }

    throw err::NotSupportedError(
        util::format("Pixel format %d is not supported", format));
}

static auto dummy = fmt::register_fmt<DbmImageDecoder>("lilim/dbm");
