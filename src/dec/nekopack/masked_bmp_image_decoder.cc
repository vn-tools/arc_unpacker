#include "dec/nekopack/masked_bmp_image_decoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "err.h"
#include "util/virtual_file_system.h"

using namespace au;
using namespace au::dec::nekopack;

bool MaskedBmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("alp");
}

res::Image MaskedBmpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto base_file = util::VirtualFileSystem::get_by_name(
        io::path(input_file.path).change_extension("bmp").name());
    if (!base_file)
        throw err::CorruptDataError("Missing base file");
    const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
    auto base_image = bmp_decoder.decode(logger, *base_file);
    res::Image mask(
        base_image.width(),
        base_image.height(),
        input_file.stream.seek(0).read_to_eof(),
        res::PixelFormat::Gray8);
    base_image.apply_mask(mask);
    return base_image;
}

static auto _ = dec::register_decoder<MaskedBmpImageDecoder>(
    "nekopack/masked-bmp");
