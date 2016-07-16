#include "dec/alice_soft/dcf_image_decoder.h"
#include "dec/alice_soft/qnt_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic1 = "dcf\x20"_b;
static const bstr magic2 = "dfdl"_b;
static const bstr magic3 = "dcgd"_b;

bool DcfImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic1.size()) == magic1;
}

res::Image DcfImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic1.size());

    const auto header1_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(header1_size);

    if (input_file.stream.read(magic2.size()) != magic2)
        throw err::CorruptDataError("Expected '" + magic2.str() + "' chunk.");

    const auto header2_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(header2_size);

    if (input_file.stream.read(magic3.size()) != magic3)
        throw err::CorruptDataError("Expected '" + magic3.str() + "' chunk.");

    const auto data_size = input_file.stream.read_le<u32>();
    const auto data = input_file.stream.read(data_size);

    if (input_file.stream.left())
        logger.warn("Extra data at end of the image!");

    const auto qnt_decoder = QntImageDecoder();
    io::File qnt_file("dummy.qnt", data);
    return qnt_decoder.decode(logger, qnt_file);
}

static auto _ = dec::register_decoder<DcfImageDecoder>("alice-soft/dcf");
