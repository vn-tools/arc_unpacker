#include "fmt/shiina_rio/ogv_file_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::shiina_rio;

static const bstr magic = "OGV\x00"_b;

bool OgvFileDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> OgvFileDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());
    file.io.skip(4);
    file.io.skip(4); // sort of file size
    if (file.io.read(4) != "fmt\x20"_b)
        throw err::CorruptDataError("Expected fmt chunk");
    file.io.skip(file.io.read_u32_le());

    if (file.io.read(4) != "data"_b)
        throw err::CorruptDataError("Expected data chunk");
    file.io.skip(4);
    const auto data = file.io.read_to_eof();

    auto output_file = std::make_unique<File>(file.name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<OgvFileDecoder>("shiina-rio/ogv");
