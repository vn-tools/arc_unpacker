#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "CMP1"_b;

bool CmpImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CmpImageDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());
    auto size_original = file.io.read_u32_le();
    auto size_compressed = file.io.read_u32_le();

    auto data = file.io.read(size_compressed);
    util::pack::LzssSettings settings;
    settings.position_bits = 11;
    settings.size_bits = 4;
    settings.min_match_size = 2;
    settings.initial_dictionary_pos = 2031;
    data = util::pack::lzss_decompress_bitwise(data, size_original, settings);

    auto output_file = std::make_unique<File>();
    output_file->io.write(data);
    output_file->name = file.name;
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<CmpImageDecoder>("riddle/cmp");
