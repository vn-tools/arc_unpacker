#include "fmt/alice_soft/ajp_image_decoder.h"
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

bool AjpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Image AjpImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    input_file.stream.skip(4 * 2);
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    auto jpeg_offset = input_file.stream.read_u32_le();
    auto jpeg_size = input_file.stream.read_u32_le();
    auto mask_offset = input_file.stream.read_u32_le();
    auto mask_size = input_file.stream.read_u32_le();

    input_file.stream.seek(jpeg_offset);
    auto jpeg_data = input_file.stream.read(jpeg_size);
    decrypt(jpeg_data);

    input_file.stream.seek(mask_offset);
    auto mask_data = input_file.stream.read(mask_size);
    decrypt(mask_data);

    fmt::jpeg::JpegImageDecoder jpeg_image_decoder;
    io::File jpeg_file;
    jpeg_file.stream.write(jpeg_data);
    auto image = jpeg_image_decoder.decode(jpeg_file);

    if (mask_size)
    {
        PmsImageDecoder pms_image_decoder;
        io::File mask_file;
        mask_file.stream.write(mask_data);
        const auto mask_image = pms_image_decoder.decode(mask_file);
        image.apply_mask(mask_image);
    }

    return image;
}

static auto dummy = fmt::register_fmt<AjpImageDecoder>("alice-soft/ajp");
