#include "fmt/wild_bug/wbi_file_decoder.h"
#include "fmt/wild_bug/wpx/decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1A""EX2\x00"_b;

bool WbiFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> WbiFileDecoder::decode_impl(
    io::File &input_file) const
{
    wpx::Decoder decoder(input_file.stream);
    auto data = decoder.read_compressed_section(0x02);

    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("dat");
    return output_file;
}

static auto dummy = fmt::register_fmt<WbiFileDecoder>("wild-bug/wbi");
