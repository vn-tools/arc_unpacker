#include "fmt/kid/cps_file_decoder.h"
#include "fmt/kid/lnd_file_decoder.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::kid;

static const bstr magic = "CPS\x00"_b;

static bstr decrypt(const bstr &input, size_t size_compressed, size_t offset)
{
    io::MemoryStream input_stream(input);
    io::MemoryStream output_stream;
    output_stream.reserve(input_stream.size());

    auto real_offset = offset - 16;
    input_stream.seek(real_offset);
    u32 key = input_stream.read_u32_le() + offset + 0x3786425;

    input_stream.seek(0);
    while (!input_stream.eof())
    {
        bool use_key = input_stream.tell() != real_offset;
        auto value = input_stream.read_u32_le();
        if (use_key)
        {
            value -= size_compressed;
            value -= key;
        }
        output_stream.write_u32_le(value);
        key = key * 0x41C64E6D + 0x9B06;
    }
    output_stream.write_u32_le(0);

    output_stream.seek(4);
    return output_stream.read_to_eof();
}

bool CpsFileDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> CpsFileDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());

    size_t size_compressed = file.stream.read_u32_le();
    auto version = file.stream.read_u16_le();
    auto compression_type = file.stream.read_u16_le();
    size_t size_original = file.stream.read_u32_le();

    auto data = file.stream.read(size_compressed - 16 - 4);

    auto offset = file.stream.read_u32_le() - 0x7534682;
    if (offset)
        data = decrypt(data, size_compressed, offset);

    if (compression_type & 1)
        data = LndFileDecoder::decompress_raw_data(data, size_original);

    auto output_file = std::make_unique<File>();
    output_file->stream.write(data);
    output_file->name = file.name;
    output_file->change_extension("prt");
    return output_file;
}

static auto dummy = fmt::register_fmt<CpsFileDecoder>("kid/cps");
