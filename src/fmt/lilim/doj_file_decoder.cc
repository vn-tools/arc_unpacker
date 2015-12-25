#include "fmt/lilim/doj_file_decoder.h"
#include "err.h"
#include "fmt/lilim/common.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic1 = "CC"_b;
static const bstr magic2 = "DD"_b;

bool DojFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("doj")
        && input_file.stream.read(magic1.size()) == magic1;
}

std::unique_ptr<io::File> DojFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic1.size());
    const auto meta_size = input_file.stream.read_u16_le() * 6;
    input_file.stream.skip(meta_size);
    if (input_file.stream.read(magic2.size()) != magic2)
        throw err::CorruptDataError("Corrupt metadata");

    input_file.stream.skip(2);
    const auto size_comp = input_file.stream.read_u32_le();
    const auto size_orig = input_file.stream.read_u32_le();
    const auto data = sysd_decompress(input_file.stream.read(size_comp));
    if (data.size() != size_orig)
        throw err::BadDataSizeError();
    return std::make_unique<io::File>(input_file.path, data);
}

static auto dummy = fmt::register_fmt<DojFileDecoder>("lilim/doj");
