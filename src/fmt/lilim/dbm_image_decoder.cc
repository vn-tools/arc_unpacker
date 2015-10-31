#include "fmt/lilim/dbm_image_decoder.h"
#include "err.h"
#include "util/format.h"
#include "util/pack/lzss.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "DM"_b;

bool DbmImageDecoder::is_recognized_impl(File &file) const
{
    file.io.seek(0);
    if (file.io.read(magic.size()) != magic)
        return false;
    file.io.skip(2);
    return file.io.read_u32_le() == file.io.size();
}

pix::Grid DbmImageDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size() + 8);
    const auto width = file.io.read_u16_le();
    const auto height = file.io.read_u16_le();
    const auto format = file.io.read_u32_le();
    if (file.io.read_u16_le() != 1)
        throw err::CorruptDataError("Expected '1'");
    const auto size_comp = file.io.read_u32_le();

    file.io.skip(1);
    if (file.io.read_u32_le() != size_comp)
        throw err::CorruptDataError("Expected compressed size");
    const size_t size_orig = file.io.read_u32_le();
    const size_t size_orig_expected = width * height * 3;
    if (size_orig != size_orig_expected)
        throw err::CorruptDataError("Size doesn't match expected target size");

    auto data = file.io.read(size_comp - 9);
    data = util::pack::lzss_decompress_bytewise(data, size_orig);

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
        image.apply_alpha_from_mask(mask);
        image.flip_vertically();
        return image;
    }

    throw err::NotSupportedError(
        util::format("Pixel format %d is not supported", format));
}

static auto dummy = fmt::register_fmt<DbmImageDecoder>("lilim/dbm");
