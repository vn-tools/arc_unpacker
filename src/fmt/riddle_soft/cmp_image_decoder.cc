#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "io/memory_stream.h"
#include "util/pack/lzss.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "CMP1"_b;

bool CmpImageDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> CmpImageDecoder::decode_impl(File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto size_original = input_file.stream.read_u32_le();
    auto size_compressed = input_file.stream.read_u32_le();

    auto data = input_file.stream.read(size_compressed);
    util::pack::LzssSettings settings;
    settings.position_bits = 11;
    settings.size_bits = 4;
    settings.min_match_size = 2;
    settings.initial_dictionary_pos = 2031;
    data = util::pack::lzss_decompress_bitwise(data, size_original, settings);

    auto output_file = std::make_unique<File>();
    output_file->stream.write(data);
    output_file->name = input_file.name;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<CmpImageDecoder>("riddle-soft/cmp");
