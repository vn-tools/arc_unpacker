#include "fmt/crowd/cwl_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "fmt/crowd/cwd_image_decoder.h"

using namespace au;
using namespace au::fmt::crowd;

static const auto magic = "SZDD"_b;

bool CwlImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic
        && input_file.path.has_extension("cwl");
}

res::Image CwlImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(10);
    const auto size_orig = input_file.stream.read_u32_le();

    algo::pack::BytewiseLzssSettings settings;
    settings.initial_dictionary_pos = 0xFF0;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), size_orig, settings);

    const fmt::crowd::CwdImageDecoder cwd_decoder;
    io::File cwd_file("dummy.cwd", data);
    return cwd_decoder.decode(cwd_file);
}

static auto dummy = fmt::register_fmt<CwlImageDecoder>("crowd/cwl");
