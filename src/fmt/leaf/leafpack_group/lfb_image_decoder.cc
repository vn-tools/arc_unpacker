#include "fmt/leaf/leafpack_group/lfb_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"
#include "fmt/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::fmt::leaf;

bool LfbImageDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("lfb");
}

pix::Grid LfbImageDecoder::decode_impl(File &file) const
{
    file.stream.seek(0);
    const auto size_orig = file.stream.read_u32_le();
    const auto data
        = common::custom_lzss_decompress(file.stream.read_to_eof(), size_orig);
    const auto pseudo_file = std::make_unique<File>(file.name, data);
    return fmt::microsoft::BmpImageDecoder().decode(*pseudo_file);
}

static auto dummy = fmt::register_fmt<LfbImageDecoder>("leaf/lfb");
