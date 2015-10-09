#include "fmt/wild_bug/wbi_file_decoder.h"
#include "fmt/wild_bug/wpx/decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "WPX\x1A""EX2\x00"_b;

bool WbiFileDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> WbiFileDecoder::decode_impl(File &file) const
{
    wpx::Decoder decoder(file.io);
    auto data = decoder.read_compressed_section(0x02);

    auto output_file = std::make_unique<File>();
    output_file->io.write(data);
    output_file->name = file.name;
    output_file->change_extension("dat");
    return output_file;
}

static auto dummy = fmt::Registry::add<WbiFileDecoder>("wild-bug/wbi");
