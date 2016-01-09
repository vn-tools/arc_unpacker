#include "dec/crowd/cwl_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/crowd/cwd_image_decoder.h"

using namespace au;
using namespace au::dec::crowd;

static const auto magic = "SZDD"_b;

bool CwlImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic
        && input_file.path.has_extension("cwl");
}

res::Image CwlImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(10);
    const auto size_orig = input_file.stream.read_le<u32>();

    algo::pack::BytewiseLzssSettings settings;
    settings.initial_dictionary_pos = 0xFF0;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), size_orig, settings);

    const auto cwd_decoder = dec::crowd::CwdImageDecoder();
    io::File cwd_file("dummy.cwd", data);
    return cwd_decoder.decode(logger, cwd_file);
}

static auto _ = dec::register_decoder<CwlImageDecoder>("crowd/cwl");
