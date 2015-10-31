#include "fmt/lilim/doj_file_decoder.h"
#include "err.h"
#include "fmt/lilim/common.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic1 = "CC"_b;
static const bstr magic2 = "DD"_b;

bool DojFileDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("doj") && file.io.read(magic1.size()) == magic1;
}

std::unique_ptr<File> DojFileDecoder::decode_impl(File &file) const
{
    file.io.seek(magic1.size());
    const auto meta_size = file.io.read_u16_le() * 6;
    file.io.skip(meta_size);
    if (file.io.read(magic2.size()) != magic2)
        throw err::CorruptDataError("Corrupt metadata");

    file.io.skip(2);
    const auto size_comp = file.io.read_u32_le();
    const auto size_orig = file.io.read_u32_le();
    const auto data = sysd_decompress(file.io.read(size_comp));
    if (data.size() != size_orig)
        throw err::BadDataSizeError();
    return std::make_unique<File>(file.name, data);
}

static auto dummy = fmt::register_fmt<DojFileDecoder>("lilim/doj");
