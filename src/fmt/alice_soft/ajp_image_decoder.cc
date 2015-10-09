#include "fmt/alice_soft/ajp_image_decoder.h"
#include <algorithm>
#include "fmt/alice_soft/pms_image_decoder.h"
#include "fmt/jpeg/jpeg_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "AJP\x00"_b;
static const bstr key =
    "\x5D\x91\xAE\x87\x4A\x56\x41\xCD\x83\xEC\x4C\x92\xB5\xCB\x16\x34"_b;

static void decrypt(bstr &input)
{
    for (auto i : util::range(std::min(input.size(), key.size())))
        input[i] ^= key[i];
}

bool AjpImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid AjpImageDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());
    file.io.skip(4 * 2);
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto jpeg_offset = file.io.read_u32_le();
    auto jpeg_size = file.io.read_u32_le();
    auto mask_offset = file.io.read_u32_le();
    auto mask_size = file.io.read_u32_le();

    file.io.seek(jpeg_offset);
    auto jpeg_data = file.io.read(jpeg_size);
    decrypt(jpeg_data);

    file.io.seek(mask_offset);
    auto mask_data = file.io.read(mask_size);
    decrypt(mask_data);

    fmt::jpeg::JpegImageDecoder jpeg_image_decoder;
    File jpeg_file;
    jpeg_file.io.write(jpeg_data);
    auto jpeg_pixels = jpeg_image_decoder.decode(jpeg_file);

    if (mask_size)
    {
        PmsImageDecoder pms_image_decoder;
        File mask_file;
        mask_file.io.write(mask_data);
        auto mask_pixels = pms_image_decoder.decode(mask_file);

        for (auto y : util::range(height))
        for (auto x : util::range(width))
        {
            jpeg_pixels.at(x, y).a = mask_pixels.at(x, y).r;
        }
    }

    return jpeg_pixels;
}

static auto dummy = fmt::register_fmt<AjpImageDecoder>("alice/ajp");
