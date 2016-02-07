#include "dec/malie/mgf_image_decoder.h"
#include "dec/png/png_image_decoder.h"

using namespace au;
using namespace au::dec::malie;

static const bstr magic = "MalieGF\x00"_b;

bool MgfImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image MgfImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    io::File pseudo_file("dummy.png", input_file.stream.seek(0).read_to_eof());
    pseudo_file.stream.seek(0).write("\x89PNG\x0D\x0A\x1A\x0A"_b);
    return dec::png::PngImageDecoder().decode(logger, pseudo_file);
}

static auto _ = dec::register_decoder<MgfImageDecoder>("malie/mgf");
