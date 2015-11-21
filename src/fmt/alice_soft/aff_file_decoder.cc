#include "fmt/alice_soft/aff_file_decoder.h"
#include "util/range.h"

// Doesn't encode anything, just wraps real files.

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "AFF\x00"_b;
static const bstr key =
    "\xC8\xBB\x8F\xB7\xED\x43\x99\x4A\xA2\x7E\x5B\xB0\x68\x18\xF8\x88"_b;

bool AffFileDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> AffFileDecoder::decode_impl(File &file) const
{
    file.stream.seek(magic.size() + 4);
    const auto size = file.stream.read_u32_le();
    file.stream.skip(4);

    auto data = file.stream.read_to_eof();
    for (const auto i : util::range(std::min<size_t>(data.size(), 64)))
        data[i] ^= key[i % key.size()];
    auto output_file = std::make_unique<File>(file.name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<AffFileDecoder>("alice-soft/aff");
